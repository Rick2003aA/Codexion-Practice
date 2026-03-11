/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shinnunohisashiryuuichi <shinnunohisash    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/07 14:11:32 by rtsubuku          #+#    #+#             */
/*   Updated: 2026/03/11 15:48:32 by shinnunohis      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	coder_do_debug(t_coder *coder)
{
	log_state(coder->sim, coder->coder_id, "is debugging");
	sleep_us(coder->sim->rules.time_to_debug * 1000LL);
}

void	coder_do_refactor(t_coder *coder)
{
	log_state(coder->sim, coder->coder_id, "is refactoring");
	sleep_us(coder->sim->rules.time_to_refactor * 1000LL);
}

static void	coder_init_cycle(t_coder *coder, int *first, int *second)
{
	int	idx;
	int	left;
	int	right;

	pthread_mutex_lock(&coder->action_mutex);
	coder->compile_count = 0;
	pthread_mutex_unlock(&coder->action_mutex);
	idx = coder->coder_id - 1;
	left = idx;
	right = (idx + 1) % coder->sim->dongle_count;
	if (left < right)
	{
		*first = left;
		*second = right;
	}
	else
	{
		*first = right;
		*second = left;
	}
}

void	*coder_routine(void *arg)
{
	int		i;
	int		first;
	int		second;
	t_coder	*coder;

	i = 0;
	coder = (t_coder *)arg;
	coder_init_cycle(coder, &first, &second);
	while (!sim_should_stop(coder->sim))
	{
		if (i % 3 == 0 && !coder_do_compile(coder, first, second))
			break ;
		else if (i % 3 == 1)
			coder_do_debug(coder);
		else if (i % 3 == 2)
			coder_do_refactor(coder);
		i++;
	}
	return (NULL);
}
