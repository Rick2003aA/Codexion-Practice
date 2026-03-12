/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shinnunohisashiryuuichi <shinnunohisash    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/09 11:54:02 by rtsubuku          #+#    #+#             */
/*   Updated: 2026/03/12 00:00:00 by shinnunohis      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_my_turn(t_sim *sim, t_dongle *d, t_coder *me)
{
	int	i;

	if (sim->rules.scheduler == FIFO)
		return (d->waiters[0] == me);
	i = 0;
	while (i < d->waiter_count)
	{
		if (d->waiters[i] != me
			&& (d->waiters[i]->next_deadline_us < me->next_deadline_us
				|| (d->waiters[i]->next_deadline_us == me->next_deadline_us
					&& d->waiters[i]->coder_id < me->coder_id)))
			return (0);
		i++;
	}
	return (1);
}

static void	remove_waiter(t_dongle *d, t_coder *coder)
{
	int	i;

	i = 0;
	while (i < d->waiter_count)
	{
		if (d->waiters[i] == coder)
		{
			d->waiters[i] = d->waiters[d->waiter_count - 1];
			d->waiter_count--;
			return ;
		}
		i++;
	}
}

int	dongle_lock(t_sim *sim, int idx, t_coder *coder)
{
	t_dongle		*d;
	long long		now;
	struct timespec	ts;

	d = &sim->dongles[idx];
	pthread_mutex_lock(&d->m);
	d->waiters[d->waiter_count++] = coder;
	while (!sim_should_stop(sim))
	{
		now = timestamp_us(sim);
		if (!d->locked && now >= d->available_at_us
			&& is_my_turn(sim, d, coder))
		{
			d->locked = 1;
			remove_waiter(d, coder);
			pthread_mutex_unlock(&d->m);
			return (1);
		}
		if (now < d->available_at_us)
			ts = us_to_abs_timespec(now_us() + d->available_at_us - now);
		else
			ts = us_to_abs_timespec(now_us() + 1000);
		pthread_cond_timedwait(&d->cv, &d->m, &ts);
	}
	remove_waiter(d, coder);
	pthread_mutex_unlock(&d->m);
	return (0);
}

void	dongle_unlock_with_cooldown(t_sim *sim, int idx)
{
	t_dongle	*d;

	d = &sim->dongles[idx];
	pthread_mutex_lock(&d->m);
	d->locked = 0;
	d->available_at_us = timestamp_us(sim)
		+ sim->rules.dongle_cooldown_ms * 1000LL;
	pthread_cond_broadcast(&d->cv);
	pthread_mutex_unlock(&d->m);
}
