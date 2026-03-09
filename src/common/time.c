/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rtsubuku <rtsubuku@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/06 13:27:11 by shinnunohis       #+#    #+#             */
/*   Updated: 2026/03/09 15:29:43 by rtsubuku         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	timestamp_ms(t_sim *sim)
{
	return (now_ms() - sim->start_ms);
}

long	now_ms(void) // Can't understand of this calculation logic
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000L + tv.tv_usec / 1000L);
}

struct timespec	ms_to_abs_timespec(long abs_ms)
{
	struct timespec	ts;

	ts.tv_sec = abs_ms / 1000;
	ts.tv_nsec = (abs_ms % 1000) * 1000000L;
	return (ts);
}

void	coder_touch(t_coder *c)
{
	pthread_mutex_lock(&c->action_mutex);
	c->last_compile_start_ms = timestamp_ms(c->sim);
	pthread_mutex_unlock(&c->action_mutex);
}
