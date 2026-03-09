/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rtsubuku <rtsubuku@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/09 11:54:02 by rtsubuku          #+#    #+#             */
/*   Updated: 2026/03/09 12:02:21 by rtsubuku         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	dongle_lock(t_sim *sim, int idx)
{
	t_dongle		*d;
	long long		now;
	long long		wait_ms;
	long long		abs_deadline_ms;
	struct timespec	ts;

	d = &sim->dongles[idx];
	pthread_mutex_lock(&d->m);
	while (!sim_should_stop(sim))
	{
		now = timestamp_ms(sim);
		if (now >= d->availble_at_ms - now)
			return (1);
		wait_ms = d->availble_at_ms - now;
		abs_deadline_ms = now_ms() + wait_ms;
		ts = ms_to_abs_timespec(abs_deadline_ms);
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
	now = timestamp_ms(sim);
	d->availble_at_ms = now + sim->rules.dongle_cooldown_ms;
	pthread_cond_broadcast(&d->cv);
	pthread_mutex_unlock(&d->m);
}
