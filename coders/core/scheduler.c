/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rtsubuku <rtsubuku@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/09 12:29:35 by rtsubuku          #+#    #+#             */
/*   Updated: 2026/03/09 17:05:00 by rtsubuku         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	heap_push(t_sim *sim, t_coder *coder)
{
	int	idx;

	idx = sim->heap_size;
	sim->compile_heap[idx] = coder;
	coder->queue_index = idx;
	sim->heap_size += 1;
	heapify_up(sim, idx);
}

static void	heap_remove(t_sim *sim, t_coder *coder)
{
	int	idx;
	int	last;

	idx = coder->queue_index;
	if (idx < 0 || idx >= sim->heap_size)
		return ;
	last = sim->heap_size - 1;
	if (idx != last)
	{
		sim->compile_heap[idx] = sim->compile_heap[last];
		sim->compile_heap[idx]->queue_index = idx;
	}
	sim->heap_size -= 1;
	coder->queue_index = -1;
	if (idx < sim->heap_size)
	{
		heapify_down(sim, idx);
		heapify_up(sim, idx);
	}
}

static int	heap_top_is(t_sim *sim, t_coder *coder)
{
	return (sim->heap_size > 0 && sim->compile_heap[0] == coder);
}

int	scheduler_wait_turn(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	pthread_mutex_lock(&sim->sched_mutex);
	if (coder->queue_index < 0)
	{
		coder->enqueue_order = sim->fifo_next_ticket++;
		coder->waiting_compile = 1;
		heap_push(sim, coder);
	}
	while (!sim_should_stop(sim)
		&& (sim->sched_active || !heap_top_is(sim, coder)))
		pthread_cond_wait(&sim->sched_cv, &sim->sched_mutex);
	if (sim_should_stop(sim))
	{
		heap_remove(sim, coder);
		coder->waiting_compile = 0;
		pthread_mutex_unlock(&sim->sched_mutex);
		return (0);
	}
	sim->sched_active = 1;
	pthread_mutex_unlock(&sim->sched_mutex);
	return (1);
}

void	scheduler_release_turn(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	pthread_mutex_lock(&sim->sched_mutex);
	heap_remove(sim, coder);
	coder->waiting_compile = 0;
	sim->sched_active = 0;
	pthread_cond_broadcast(&sim->sched_cv);
	pthread_mutex_unlock(&sim->sched_mutex);
}
