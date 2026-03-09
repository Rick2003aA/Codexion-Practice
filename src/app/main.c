/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rtsubuku <rtsubuku@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/07 11:03:37 by rtsubuku          #+#    #+#             */
/*   Updated: 2026/03/09 15:00:44 by rtsubuku         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	setup_sim(t_sim *sim, t_rules *rules, int ac, char **av)
{
	if (parse_args(rules, ac, av))
		return (1);
	sim->rules = *rules;
	sim->coder_count = rules->number_of_coders;
	sim->dongle_count = rules->number_of_coders;
	return (sim_init(sim));
}

static int	init_coders(t_sim *sim, t_rules *rules)
{
	int		i;
	t_coder	*coders;

	coders = sim->coders;
	i = 0;
	while (i < sim->coder_count)
	{
		coders[i].coder_id = i + 1;
		coders[i].sim = sim;
		// Roll back already initialized coder mutexes on partial failure.
		if (pthread_mutex_init(&coders[i].action_mutex, NULL) != 0)
		{
			while (i > 0)
				pthread_mutex_destroy(&coders[--i].action_mutex);
			return (1);
		}
		coders[i].last_compile_start_ms = 0;
		coders[i].waiting_compile = 0;
		coders[i].fifo_ticket = -1;
		coders[i].enqueue_order = 0;
		coders[i].queue_index = -1;
		coders[i].next_deadline_ms = rules->time_to_burnout;
		i++;
	}
	return (0);
}

static int	run_simulation(t_sim *sim, pthread_t *monitor_th,
		int *created_workers, int *monitor_created)
{
	int		i;
	t_coder	*coders;

	*created_workers = 0;
	*monitor_created = 0;
	coders = sim->coders;
	// Start monitor first so it can observe worker lifecycle immediately.
	if (pthread_create(monitor_th, NULL, monitor_routine, sim) != 0)
		return (1);
	*monitor_created = 1;
	i = 0;
	while (i < sim->coder_count)
	{
		// Keep count of successfully created workers for safe cleanup.
		if (pthread_create(&sim->threads[i], NULL, coder_routine, &coders[i]) != 0)
		{
			*created_workers = i;
			return (1);
		}
		i++;
	}
	*created_workers = i;
	return (0);
}

static void	cleanup_sim(t_sim *sim, pthread_t monitor_th)
{
	int		i;
	t_coder	*coders;

	coders = sim->coders;
	i = 0;
	while (i < sim->coder_count)
		pthread_join(sim->threads[i++], NULL);
	pthread_join(monitor_th, NULL);
	i = 0;
	while (i < sim->coder_count)
		pthread_mutex_destroy(&coders[i++].action_mutex);
	sim_destroy(sim);
	free(sim->threads);
	free(sim->coders);
}

static void	cleanup_sim_after_failed_run(t_sim *sim, pthread_t monitor_th,
		int created_workers, int monitor_created)
{
	int		i;
	t_coder	*coders;

	coders = sim->coders;
	// Notify all waiters to unblock before joining partially started threads.
	sim_request_stop(sim);
	i = 0;
	while (i < created_workers)
		pthread_join(sim->threads[i++], NULL);
	if (monitor_created)
		pthread_join(monitor_th, NULL);
	i = 0;
	while (i < sim->coder_count)
		pthread_mutex_destroy(&coders[i++].action_mutex);
	sim_destroy(sim);
	free(sim->threads);
	free(sim->coders);
}

int	main(int ac, char **av)
{
	t_sim		sim;
	t_rules		rules;
	pthread_t	monitor_th;
	int			created_workers;
	int			monitor_created;

	if (setup_sim(&sim, &rules, ac, av))
		return (1);
	// If coder mutex initialization fails, only sim-level resources exist yet.
	if (init_coders(&sim, &rules))
	{
		sim_destroy(&sim);
		free(sim.threads);
		free(sim.coders);
		return (1);
	}
	// If thread creation fails mid-way, clean up only started threads safely.
	if (run_simulation(&sim, &monitor_th, &created_workers, &monitor_created))
	{
		cleanup_sim_after_failed_run(&sim, monitor_th,
			created_workers, monitor_created);
		return (1);
	}
	cleanup_sim(&sim, monitor_th);
	return (0);
}
