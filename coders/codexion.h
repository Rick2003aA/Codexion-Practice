/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shinnunohisashiryuuichi <shinnunohisash    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/07 11:04:57 by rtsubuku          #+#    #+#             */
/*   Updated: 2026/03/11 16:39:20 by shinnunohis      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <pthread.h>

typedef struct s_coder	t_coder;

typedef enum e_scheduler
{
	FIFO,
	EDF
}	t_scheduler;

typedef struct s_dongle
{
	pthread_mutex_t	m;
	pthread_cond_t	cv;
	long long		available_at_us;
	int				locked;
	t_coder			*waiters[2];
	int				waiter_count;
}	t_dongle;

typedef struct s_rules
{
	int			number_of_coders;
	int			time_to_burnout;
	int			time_to_compile;
	int			time_to_debug;
	int			time_to_refactor;
	int			number_of_compiles_required;
	int			dongle_cooldown_ms;
	t_scheduler	scheduler;
}	t_rules;

typedef struct s_sim
{
	t_rules			rules;

	int				coder_count;
	int				dongle_count;
	t_coder			*coders;
	pthread_t		*threads;
	long long		start_us;
	pthread_mutex_t	log_mutex;
	t_dongle		*dongles;

	int				stop;
	pthread_mutex_t	stop_mutex;
}	t_sim;

typedef struct s_coder
{
	int				coder_id;
	t_sim			*sim;

	long long		last_compile_start_us;
	int				compile_count;

	pthread_mutex_t	action_mutex;

	long long		next_deadline_us;
}	t_coder;

// core/monitor_checks.c
int		coder_timed_out(t_coder *c, long long now, int timeout);
int		monitor_find_burned_out(t_sim *sim, long long now);
int		all_compiled_enough(t_sim *sim);

// core/monitor.c
void	*monitor_routine(void *arg);

// core/sim_stop.c
int		sim_should_stop(t_sim *sim);
void	sim_request_stop(t_sim *sim);

// core/dongle.c
int		dongle_lock(t_sim *sim, int idx, t_coder *coder);
void	dongle_unlock_with_cooldown(t_sim *sim, int idx);

// core/coder_actions.c
int		coder_do_compile(t_coder *coder, int first, int second);

// core/coder_routine.c
void	coder_do_debug(t_coder *coder);
void	coder_do_refactor(t_coder *coder);
void	*coder_routine(void *arg);

// common/cleanup.c
void	sim_destroy(t_sim *sim);
void	cleanup_threads_coders(t_sim *sim);
void	cleanup_sim(t_sim *sim, pthread_t monitor_th);
void	cleanup_sim_after_failed_run(t_sim *sim, pthread_t monitor_th,
		int created_workers, int monitor_created);

// common/sleep.c
void	sleep_us(long long us);
void	sleep_us_interruptible(t_sim *sim, long long us);

// log.c
void	log_state(t_sim *sim, int coder_id, const char *msg);
void	log_state_at(t_sim *sim, int coder_id, long long timestamp_us,
			const char *msg);

// time.c
long long	now_us(void);
void		coder_touch_at(t_coder *c, long long timestamp_us);
long long	timestamp_us(t_sim *sim);
struct timespec	us_to_abs_timespec(long long abs_us);

// sim_init.c
int		sim_init(t_sim *sim);

// parse.c
int		parse_args(t_rules *rules, int ac, char **av);

#endif
