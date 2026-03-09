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

static void	init_coder_fields(t_coder *coders, t_rules *rules, int i, t_sim *sim)
{
	coders[i].coder_id = i + 1;
	coders[i].sim = sim;
	coders[i].last_compile_start_ms = 0;
	coders[i].waiting_compile = 0;
	coders[i].fifo_ticket = -1;
	coders[i].enqueue_order = 0;
	coders[i].queue_index = -1;
	coders[i].next_deadline_ms = rules->time_to_burnout;
}

static int	init_coders(t_sim *sim, t_rules *rules)
{
	int		i;
	t_coder	*coders;

	coders = sim->coders;
	i = 0;
	while (i < sim->coder_count)
	{
		init_coder_fields(coders, rules, i, sim);
		if (pthread_mutex_init(&coders[i].action_mutex, NULL) != 0)
		{
			while (i > 0)
				pthread_mutex_destroy(&coders[--i].action_mutex);
			return (1);
		}
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
	if (pthread_create(monitor_th, NULL, monitor_routine, sim) != 0)
		return (1);
	*monitor_created = 1;
	i = 0;
	while (i < sim->coder_count)
	{
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

int	main(int ac, char **av)
{
	t_sim		sim;
	t_rules		rules;
	pthread_t	monitor_th;
	int			created_workers;
	int			monitor_created;

	if (setup_sim(&sim, &rules, ac, av))
		return (1);
	if (init_coders(&sim, &rules))
	{
		sim_destroy(&sim);
		free(sim.threads);
		free(sim.coders);
		return (1);
	}
	if (run_simulation(&sim, &monitor_th, &created_workers, &monitor_created))
	{
		cleanup_sim_after_failed_run(&sim, monitor_th,
			created_workers, monitor_created);
		return (1);
	}
	cleanup_sim(&sim, monitor_th);
	return (0);
}
