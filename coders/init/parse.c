/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shinnunohisashiryuuichi <shinnunohisash    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/07 11:42:39 by rtsubuku          #+#    #+#             */
/*   Updated: 2026/03/11 15:48:32 by shinnunohis      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <limits.h>

int	ft_isdigit_str(char *str)
{
	int	i;

	if (!str || str[0] == '\0')
		return (0);
	i = 0;
	while (str[i])
	{
		if (!('0' <= str[i] && str[i] <= '9'))
			return (0);
		i++;
	}
	return (1);
}

static int	ft_safe_atoi(const char *str, int *out)
{
	long	val;

	val = 0;
	while (*str)
	{
		val = val * 10 + (*str - '0');
		if (val > INT_MAX)
			return (1);
		str++;
	}
	*out = (int)val;
	return (0);
}

static int	parse_numeric_rules(t_rules *rules, char **av)
{
	int		vals[7];
	int		i;

	i = 0;
	while (i < 7)
	{
		if (ft_safe_atoi(av[i + 1], &vals[i]))
			return (1);
		i++;
	}
	rules->number_of_coders = vals[0];
	rules->time_to_burnout = vals[1];
	rules->time_to_compile = vals[2];
	rules->time_to_debug = vals[3];
	rules->time_to_refactor = vals[4];
	rules->number_of_compiles_required = vals[5];
	rules->dongle_cooldown_ms = vals[6];
	if (vals[0] < 1 || vals[1] < 1 || vals[2] < 1 || vals[3] < 1
		|| vals[4] < 1 || vals[5] < 1 || vals[6] < 0)
		return (1);
	return (0);
}

static int	parse_scheduler_rule(t_rules *rules, char *scheduler)
{
	if (strcmp(scheduler, "fifo") == 0)
		rules->scheduler = FIFO;
	else if (strcmp(scheduler, "edf") == 0)
		rules->scheduler = EDF;
	else
		return (1);
	return (0);
}

int	parse_args(t_rules *rules, int ac, char **av)
{
	int	i;

	if (ac != 9)
		return (1);
	i = 1;
	while (i < 8)
		if (!ft_isdigit_str(av[i++]))
			return (1);
	if (parse_numeric_rules(rules, av))
		return (1);
	return (parse_scheduler_rule(rules, av[8]));
}
