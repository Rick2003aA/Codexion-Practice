/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor_checks.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shinnunohisashiryuuichi <shinnunohisash    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/07 13:48:07 by rtsubuku          #+#    #+#             */
/*   Updated: 2026/03/11 15:48:32 by shinnunohis      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
// 0: continue
// 1: stop

int	coder_timed_out(t_coder *c, long long now, int timeout)
{
	long long	last;

	pthread_mutex_lock(&c->action_mutex);
	last = c->last_compile_start_us;
	pthread_mutex_unlock(&c->action_mutex);
	return (now - last > timeout * 1000LL);
}

int	monitor_find_burned_out(t_sim *sim, long long now)
{
	int	i;

	i = 0;
	while (i < sim->coder_count)
	{
		if (coder_timed_out(&sim->coders[i], now, sim->rules.time_to_burnout))
			return (sim->coders[i].coder_id);
		i++;
	}
	return (0);
}

int	all_compiled_enough(t_sim *sim)
{
	int	i;
	int	ok;

	if (sim->rules.number_of_compiles_required <= 0)
		return (0);
	i = 0;
	while (i < sim->coder_count)
	{
		pthread_mutex_lock(&sim->coders[i].action_mutex);
		ok = (sim->coders[i].compile_count
				>= sim->rules.number_of_compiles_required);
		pthread_mutex_unlock(&sim->coders[i].action_mutex);
		if (!ok)
			return (0);
		i++;
	}
	return (1);
}
