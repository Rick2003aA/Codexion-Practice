/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shinnunohisashiryuuichi <shinnunohisash    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/09 12:29:35 by rtsubuku          #+#    #+#             */
/*   Updated: 2026/03/10 14:41:34 by shinnunohis      ###   ########.fr       */
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

void	heapify_up(t_sim *sim, int idx)
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

void	heapify_down(t_sim *sim, int idx)
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
