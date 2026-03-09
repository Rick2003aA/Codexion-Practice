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

static int	is_higher_priority(t_sim *sim, t_coder *a, t_coder *b)
{
	if (sim->rules.scheduler == FIFO)
	{
		if (a->enqueue_order != b->enqueue_order)
			return (a->enqueue_order < b->enqueue_order);
	}
	else
	{
		if (a->next_deadline_ms != b->next_deadline_ms)
			return (a->next_deadline_ms < b->next_deadline_ms);
	}
	return (a->coder_id < b->coder_id);
}

static void	swap_heap_nodes(t_sim *sim, int i, int j)
{
	t_coder	*tmp;

	tmp = sim->compile_heap[i];
	sim->compile_heap[i] = sim->compile_heap[j];
	sim->compile_heap[j] = tmp;
	sim->compile_heap[i]->queue_index = i;
	sim->compile_heap[j]->queue_index = j;
}

static void	heapify_up(t_sim *sim, int idx)
{
	int	parent;

	while (idx > 0)
	{
		parent = (idx - 1) / 2;
		if (!is_higher_priority(sim, sim->compile_heap[idx],
				sim->compile_heap[parent]))
			break ;
		swap_heap_nodes(sim, idx, parent);
		idx = parent;
	}
}

static void	heapify_down(t_sim *sim, int idx)
{
	int	left;
	int	right;
	int	best;

	while (1)
	{
		left = idx * 2 + 1;
		right = left + 1;
		best = idx;
		if (left < sim->heap_size
			&& is_higher_priority(sim, sim->compile_heap[left],
				sim->compile_heap[best]))
			best = left;
		if (right < sim->heap_size
			&& is_higher_priority(sim, sim->compile_heap[right],
				sim->compile_heap[best]))
			best = right;
		if (best == idx)
			break ;
		swap_heap_nodes(sim, idx, best);
		idx = best;
	}
}

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
	while (!sim_should_stop(sim) && !heap_top_is(sim, coder))
		pthread_cond_wait(&sim->sched_cv, &sim->sched_mutex);
	if (sim_should_stop(sim))
	{
		heap_remove(sim, coder);
		coder->waiting_compile = 0;
		pthread_mutex_unlock(&sim->sched_mutex);
		return (0);
	}
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
	pthread_cond_broadcast(&sim->sched_cv);
	pthread_mutex_unlock(&sim->sched_mutex);
}
