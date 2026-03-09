/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rtsubuku <rtsubuku@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/07 14:11:32 by rtsubuku          #+#    #+#             */
/*   Updated: 2026/03/09 11:44:29 by rtsubuku         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	coder_do_debug(t_coder *coder)
{
	log_state(coder->sim, coder->coder_id, "is debugging");
	sleep_ms(coder->sim->rules.time_to_debug);
}

void	coder_do_refactor(t_coder *coder)
{
	log_state(coder->sim, coder->coder_id, "is refactoring");
	sleep_ms(coder->sim->rules.time_to_refactor);
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
