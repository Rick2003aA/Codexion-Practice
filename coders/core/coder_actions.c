/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_actions.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shinnunohisashiryuuichi <shinnunohisash    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/06 13:27:11 by shinnunohis       #+#    #+#             */
/*   Updated: 2026/03/11 15:48:15 by shinnunohis      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	coder_take_dongles(t_coder *coder, int first, int second)
{
	t_sim	*sim;

	sim = coder->sim;
	if (!dongle_lock(sim, first))
		return (0);
	log_state(sim, coder->coder_id, "has taken a dongle");
	if (!dongle_lock(sim, second))
	{
		dongle_unlock_with_cooldown(sim, first);
		return (0);
	}
	log_state(sim, coder->coder_id, "has taken a dongle");
	return (1);
}

static void	coder_finish_compile(t_coder *coder, int first, int second)
{
	t_sim	*sim;
	long long	compile_start_us;

	sim = coder->sim;
	compile_start_us = timestamp_us(sim);
	coder_touch_at(coder, compile_start_us);
	pthread_mutex_lock(&sim->sched_mutex);
	coder->next_deadline_us = compile_start_us
		+ sim->rules.time_to_burnout * 1000LL;
	pthread_mutex_unlock(&sim->sched_mutex);
	log_state_at(sim, coder->coder_id, compile_start_us, "is compiling");
	sleep_us_interruptible(sim, sim->rules.time_to_compile * 1000LL);
	dongle_unlock_with_cooldown(sim, second);
	dongle_unlock_with_cooldown(sim, first);
	pthread_mutex_lock(&coder->action_mutex);
	coder->compile_count += 1;
	pthread_mutex_unlock(&coder->action_mutex);
}

static int	coder_handle_single(t_coder *coder, int first)
{
	if (!dongle_lock(coder->sim, first))
	{
		scheduler_release_turn(coder);
		return (0);
	}
	log_state(coder->sim, coder->coder_id, "has taken a dongle");
	scheduler_release_turn(coder);
	while (!sim_should_stop(coder->sim))
		sleep_us(1000);
	dongle_unlock_with_cooldown(coder->sim, first);
	return (0);
}

int	coder_do_compile(t_coder *coder, int first, int second)
{
	if (!scheduler_wait_turn(coder))
		return (0);
	if (first == second)
		return (coder_handle_single(coder, first));
	if (!coder_take_dongles(coder, first, second))
	{
		scheduler_release_turn(coder);
		return (0);
	}
	coder_finish_compile(coder, first, second);
	scheduler_release_turn(coder);
	return (1);
}
