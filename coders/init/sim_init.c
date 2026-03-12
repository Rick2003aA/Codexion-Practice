/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sim_init.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shinnunohisashiryuuichi <shinnunohisash    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/07 12:53:14 by rtsubuku          #+#    #+#             */
/*   Updated: 2026/03/12 00:00:00 by shinnunohis      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	cleanup_sync_objects(t_sim *sim)
{
	pthread_mutex_destroy(&sim->log_mutex);
	pthread_mutex_destroy(&sim->stop_mutex);
}

static int	init_sync_objects(t_sim *sim)
{
	if (pthread_mutex_init(&sim->stop_mutex, NULL) != 0)
		return (1);
	if (pthread_mutex_init(&sim->log_mutex, NULL) != 0)
		return (pthread_mutex_destroy(&sim->stop_mutex), 1);
	return (0);
}

static int	init_single_dongle(t_dongle *d)
{
	if (pthread_mutex_init(&d->m, NULL) != 0)
		return (1);
	if (pthread_cond_init(&d->cv, NULL) != 0)
		return (pthread_mutex_destroy(&d->m), 1);
	d->available_at_us = 0;
	d->locked = 0;
	d->waiter_count = 0;
	d->waiters[0] = NULL;
	d->waiters[1] = NULL;
	return (0);
}

static int	init_all_dongles(t_sim *sim)
{
	int	i;

	sim->dongles = malloc(sizeof(t_dongle) * sim->dongle_count);
	if (!sim->dongles)
		return (1);
	i = 0;
	while (i < sim->dongle_count && !init_single_dongle(&sim->dongles[i]))
		i++;
	if (i == sim->dongle_count)
		return (0);
	while (i > 0)
	{
		pthread_cond_destroy(&sim->dongles[i - 1].cv);
		pthread_mutex_destroy(&sim->dongles[i - 1].m);
		i--;
	}
	return (free(sim->dongles), sim->dongles = NULL, 1);
}

int	sim_init(t_sim *sim)
{
	sim->coders = malloc(sizeof(t_coder) * sim->coder_count);
	if (!sim->coders)
		return (1);
	sim->threads = malloc(sizeof(pthread_t) * sim->coder_count);
	if (!sim->threads)
		return (cleanup_threads_coders(sim), 1);
	sim->start_us = now_us();
	sim->stop = 0;
	sim->dongles = NULL;
	if (init_sync_objects(sim))
		return (cleanup_threads_coders(sim), 1);
	if (init_all_dongles(sim))
		return (cleanup_sync_objects(sim), cleanup_threads_coders(sim), 1);
	return (0);
}
