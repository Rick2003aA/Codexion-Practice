# Codexion-Practice
Codexion_practice/
├── Makefile
├── README.md
└── src/
    ├── codexion.h               # 全体共通の型定義・関数プロトタイプ
    │
    ├── app/
    │   └── main.c               # エントリーポイント・シミュレーション起動制御
    │       ├── setup_sim()
    │       ├── init_coder_fields()
    │       ├── init_coders()
    │       ├── run_simulation()
    │       └── main()
    │
    ├── core/                    # シミュレーションのコアロジック
    │   ├── heap.c               # ヒープ操作（スケジューラ内部用）
    │   │   ├── is_higher_priority()  [static]
    │   │   ├── swap_heap_nodes()     [static]
    │   │   ├── heapify_up()
    │   │   └── heapify_down()
    │   │
    │   ├── scheduler.c          # コンパイル順番管理（FIFO / EDF）
    │   │   ├── heap_push()           [static]
    │   │   ├── heap_remove()         [static]
    │   │   ├── heap_top_is()         [static]
    │   │   ├── scheduler_wait_turn()
    │   │   └── scheduler_release_turn()
    │   │
    │   ├── coder_routine.c      # コーダースレッドのメインループ
    │   │   ├── coder_do_debug()
    │   │   ├── coder_do_refactor()
    │   │   ├── coder_init_cycle()    [static]
    │   │   └── coder_routine()
    │   │
    │   ├── coder_actions.c      # コンパイルアクション実装
    │   │   ├── coder_take_dongles()   [static]
    │   │   ├── coder_finish_compile() [static]
    │   │   ├── coder_handle_single()  [static]
    │   │   └── coder_do_compile()
    │   │
    │   ├── dongle.c             # ドングル（リソース）のロック管理
    │   │   ├── dongle_lock()
    │   │   └── dongle_unlock_with_cooldown()
    │   │
    │   ├── monitor.c            # バーンアウト・終了条件の監視スレッド
    │   │   ├── monitor_check_cycle()  [static]
    │   │   └── monitor_routine()
    │   │
    │   ├── monitor_checks.c     # モニター用チェック関数群
    │   │   ├── coder_timed_out()
    │   │   ├── monitor_find_burned_out()
    │   │   └── all_compiled_enough()
    │   │
    │   └── sim_stop.c           # シミュレーション停止フラグ管理
    │       ├── sim_should_stop()
    │       └── sim_request_stop()
    │
    ├── init/                    # 初期化処理
    │   ├── sim_init.c           # シミュレーション構造体の初期化
    │   │   ├── cleanup_sync_objects() [static]
    │   │   ├── init_sync_objects()    [static]
    │   │   ├── init_single_dongle()   [static]
    │   │   ├── init_all_dongles()     [static]
    │   │   └── sim_init()
    │   │
    │   └── parse.c              # コマンドライン引数パース
    │       ├── ft_isdigit_str()
    │       ├── parse_numeric_rules()  [static]
    │       ├── parse_scheduler_rule() [static]
    │       └── parse_args()
    │
    └── common/                  # 汎用ユーティリティ
        ├── cleanup.c            # リソース解放処理
        │   ├── destroy_dongles()              [static]
        │   ├── sim_destroy()
        │   ├── cleanup_threads_coders()
        │   ├── cleanup_sim()
        │   └── cleanup_sim_after_failed_run()
        │
        ├── log.c                # ログ出力
        │   └── log_state()
        │
        ├── time.c               # 時刻計算・タッチ処理
        │   ├── timestamp_us()
        │   ├── now_us()
        │   ├── ms_to_abs_timespec()
        │   └── coder_touch()
        │
        └── sleep.c              # スリープユーティリティ
            └── sleep_us()

全体の流れ
起動フェーズ（main.c → init/）

main()
  → parse_args()          引数バリデーション・ルール構造体に格納
  → sim_init()            メモリ確保・mutex/cond初期化・ドングル初期化
  → init_coders()         各コーダー構造体のフィールド初期化
実行フェーズ（2種類のスレッド）

run_simulation()
  → pthread_create(monitor_routine)   モニタースレッド起動
  → pthread_create(coder_routine) × N  コーダースレッド起動（N人分）
コーダースレッド（coder_routine） — N本並行


coder_routine()  ← 各コーダーが独立して実行
  ループ:
    i%3==0 → coder_do_compile()     コンパイル（ドングルが必要）
    i%3==1 → coder_do_debug()       デバッグ（待つだけ）
    i%3==2 → coder_do_refactor()    リファクタ（待つだけ）
coder_do_compile() の内部フロー：


scheduler_wait_turn()    → ヒープ（FIFO/EDF）で自分の順番を待つ
dongle_lock() × 2        → 左右のドングルをロック
coder_finish_compile()   → コンパイル実行・compile_countインクリメント
dongle_unlock() × 2      → ドングル解放（クールダウン付き）
scheduler_release_turn() → 次の人に順番を譲る
モニタースレッド（monitor_routine） — 1本


monitor_routine()
  1msごとにループ:
    monitor_find_burned_out()  → 最後のコンパイルから time_to_burnout 経過？
      → Yes: log "burned out" → sim_request_stop()
    all_compiled_enough()      → 全員が必要回数コンパイル完了？
      → Yes: sim_request_stop()
終了フェーズ

sim_request_stop()
  → stop フラグを立てる
  → sched_cv・各ドングルの cv を broadcast（待機中スレッドを起こす）

全スレッドが return → pthread_join で回収
cleanup_sim() / cleanup_sim_after_failed_run()
  → mutex/cond 破棄 → free
データの依存関係（中心は t_sim）

t_sim
├── rules (t_rules)      実行パラメータ（全スレッドから読み取り）
├── coders[] (t_coder)   各コーダーの状態（action_mutex で保護）
├── dongles[] (t_dongle) 共有リソース（mutex+cond で保護）
├── compile_heap[]       スケジューラ用優先度ヒープ（sched_mutex で保護）
├── stop (int)           終了フラグ（stop_mutex で保護）
└── log_mutex            printf の排他制御
