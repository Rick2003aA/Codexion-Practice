/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shinnunohisashiryuuichi <shinnunohisash    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/09 11:54:02 by rtsubuku          #+#    #+#             */
/*   Updated: 2026/03/11 15:48:32 by shinnunohis      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	dongle_lock(t_sim *sim, int idx)
{
	t_dongle		*d;
	long long		now;
	long long		wait_us;
	long long		abs_deadline_us;
	struct timespec	ts;

	d = &sim->dongles[idx];
	pthread_mutex_lock(&d->m);
	while (!sim_should_stop(sim))
	{
		now = timestamp_us(sim);
		if (now >= d->availble_at_us)
			return (1);
		wait_us = d->availble_at_us - now;
		abs_deadline_us = now_us() + wait_us;
		ts = us_to_abs_timespec(abs_deadline_us);
		pthread_cond_timedwait(&d->cv, &d->m, &ts);
	}
	pthread_mutex_unlock(&d->m);
	return (0);
}

void	dongle_unlock_with_cooldown(t_sim *sim, int idx)
{
	t_dongle	*d;
	long long	now;

	d = &sim->dongles[idx];
	now = timestamp_us(sim);
	d->availble_at_us = now + sim->rules.dongle_cooldown_ms * 1000LL;
	pthread_cond_broadcast(&d->cv);
	pthread_mutex_unlock(&d->m);
}
