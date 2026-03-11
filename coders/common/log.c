/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rtsubuku <rtsubuku@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/06 13:27:11 by shinnunohis       #+#    #+#             */
/*   Updated: 2026/03/09 11:47:22 by rtsubuku         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	log_state(t_sim *sim, int coder_id, const char *msg)
{
	long long	timestamp;

	pthread_mutex_lock(&sim->log_mutex);
	if (!sim_should_stop(sim))
	{
		timestamp = timestamp_ms(sim);
		printf("%lld %d %s\n", timestamp, coder_id, msg);
	}
	pthread_mutex_unlock(&sim->log_mutex);
}
