/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sleep.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shinnunohisashiryuuichi <shinnunohisash    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/09 11:46:08 by rtsubuku          #+#    #+#             */
/*   Updated: 2026/03/11 15:48:32 by shinnunohis      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	sleep_us(long long us)
{
	usleep(us);
}

void	sleep_us_interruptible(t_sim *sim, long long us)
{
	long long	end;

	end = now_us() + us;
	while (!sim_should_stop(sim) && now_us() < end)
		usleep(500);
}
