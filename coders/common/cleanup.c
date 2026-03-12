/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shinnunohisashiryuuichi <shinnunohisash    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/09 14:36:30 by rtsubuku          #+#    #+#             */
/*   Updated: 2026/03/09 15:39:00 by shinnunohis      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	destroy_dongles(t_sim *sim)
{
	int	i;

	if (!sim->dongles)
		return ;
	i = 0;
	while (i < sim->dongle_count)
	{
		pthread_cond_destroy(&sim->dongles[i].cv);
		pthread_mutex_destroy(&sim->dongles[i].m);
		i++;
	}
	free(sim->dongles);
	sim->dongles = NULL;
}

void	sim_destroy(t_sim *sim)
{
	if (!sim)
		return ;
	destroy_dongles(sim);
	pthread_mutex_destroy(&sim->log_mutex);
	pthread_mutex_destroy(&sim->stop_mutex);
}

void	cleanup_threads_coders(t_sim *sim)
{
	free(sim->threads);
	sim->threads = NULL;
	free(sim->coders);
	sim->coders = NULL;
}

void	cleanup_sim(t_sim *sim, pthread_t monitor_th)
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

void	cleanup_sim_after_failed_run(t_sim *sim, pthread_t monitor_th,
		int created_workers, int monitor_created)
{
	int		i;
	t_coder	*coders;

	coders = sim->coders;
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
