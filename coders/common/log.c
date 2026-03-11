/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shinnunohisashiryuuichi <shinnunohisash    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/06 13:27:11 by shinnunohis       #+#    #+#             */
/*   Updated: 2026/03/11 15:48:32 by shinnunohis      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	log_state(t_sim *sim, int coder_id, const char *msg)
{
	log_state_at(sim, coder_id, timestamp_us(sim), msg);
}

void	log_state_at(t_sim *sim, int coder_id, long long timestamp_us,
		const char *msg)
{
	long long	timestamp_ms;

	pthread_mutex_lock(&sim->log_mutex);
	if (!sim_should_stop(sim))
	{
		timestamp_ms = timestamp_us / 1000LL;
		printf("%lld %d %s\n", timestamp_ms, coder_id, msg);
	}
	pthread_mutex_unlock(&sim->log_mutex);
}
