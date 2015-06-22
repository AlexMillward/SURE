#ifndef DIVISION_HELPERS_H
#define DIVISION_HELPERS_H

#include "header.h"

division_transport_quote calculate_transport_costs(
  division_transport_information * source_information,
  double destination_x, double destination_y, double quantity);

#endif
