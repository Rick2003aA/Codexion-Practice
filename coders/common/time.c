/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shinnunohisashiryuuichi <shinnunohisash    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/06 13:27:11 by shinnunohis       #+#    #+#             */
/*   Updated: 2026/03/11 15:48:32 by shinnunohis      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long long	timestamp_us(t_sim *sim)
{
	return (now_us() - sim->start_us);
}

long long	now_us(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((long long)tv.tv_sec * 1000000LL + tv.tv_usec);
}

struct timespec	us_to_abs_timespec(long long abs_us)
{
	struct timespec	ts;

	ts.tv_sec = abs_us / 1000000LL;
	ts.tv_nsec = (abs_us % 1000000LL) * 1000L;
	return (ts);
}

void	coder_touch_at(t_coder *c, long long timestamp_us)
{
	pthread_mutex_lock(&c->action_mutex);
	c->last_compile_start_us = timestamp_us;
	pthread_mutex_unlock(&c->action_mutex);
}
