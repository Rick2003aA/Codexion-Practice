/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rtsubuku <rtsubuku@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/07 12:45:57 by rtsubuku          #+#    #+#             */
/*   Updated: 2026/03/09 17:31:38 by rtsubuku         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	monitor_check_cycle(t_sim *sim, long long now)
{
	int	burned_id;

	burned_id = monitor_find_burned_out(sim, now);
	if (burned_id)
	{
		log_state(sim, burned_id, "burned out");
		sim_request_stop(sim);
		return (1);
	}
	if (all_compiled_enough(sim))
	{
		sim_request_stop(sim);
		return (1);
	}
	return (0);
}

void	*monitor_routine(void *arg)
{
	t_sim		*sim;
	long long	now;

	sim = (t_sim *)arg;
	while (!sim_should_stop(sim))
	{
		now = timestamp_ms(sim);
		if (monitor_check_cycle(sim, now))
			return (NULL);
		sleep_ms(1);
	}
	return (NULL);
}
