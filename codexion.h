/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rtsubuku <rtsubuku@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/07 11:04:57 by rtsubuku          #+#    #+#             */
/*   Updated: 2026/03/07 13:32:22 by rtsubuku         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <pthread.h>

typedef struct s_coder	t_coder;

typedef enum e_scheduler
{
	FIFO,
	EDF
}	t_scheduler;

typedef struct s_dongle
{
	pthread_mutex_t	m;
	pthread_cond_t	cv;
	long long		availble_at_ms;
}	t_dongle;

typedef struct s_rules
{
	int			number_of_coders;
	long long	time_to_burnout;
	long long	time_to_compile;
	long long	time_to_debug;
	long long	time_to_refactor;
	int			number_of_compiles_required;
	long long	dongle_cooldown_ms;
	t_scheduler	scheduler;
}	t_rules;

typedef struct s_sim
{
	t_rules			rules;

	int				coder_count;
	int				dongle_count;
	t_coder			*coders;
	pthread_t		*threads;
	long long		start_ms;
	pthread_mutex_t	log_mutex;
	t_dongle		*dongles;

	int				stop;
	pthread_mutex_t	stop_mutex;

	pthread_mutex_t	sched_mutex;
	pthread_cond_t	sched_cv;
	long long		fifo_next_ticket;
	long long		fifo_serving_ticket;
}	t_sim;

typedef struct s_coder
{
	int				coder_id;
	t_sim			*sim;

	long long		last_compile_start_ms;
	int				compile_count;

	pthread_mutex_t	action_mutex;

	long long		fifo_ticket;
	int				waiting_compile;
	long			next_deadline_ms;
}	t_coder;

#endif
