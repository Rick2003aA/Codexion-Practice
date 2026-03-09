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
	free(sim->compile_heap);
	sim->compile_heap = NULL;
	sim->heap_size = 0;
	sim->heap_cap = 0;
	pthread_mutex_destroy(&sim->log_mutex);
	pthread_mutex_destroy(&sim->stop_mutex);
	pthread_cond_destroy(&sim->sched_cv);
	pthread_mutex_destroy(&sim->sched_mutex);
}
