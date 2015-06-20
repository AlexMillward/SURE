#include <stdlib.h>
#include <stdio.h>

#include "header.h"
#include "Firm_agent_header.h"

double getUnitCosts(int current, division_unit_costs_array * costs) {

  // Initialise variables
  int highest_costs_id = -1;
  double highest_costs = 0;

  /*
    TODO (potential)
    Find most expensive (for safety) division capable of producing 'current', presently only the
    'current' division is considered i.e if 2 divisions with the same output but different inputs
    were considered results could not be accurate (or safe).
  */
  for (int c=0; c<costs->size; c++) {
    if (costs->array[c].good_type == current && costs->array[c].unit_costs > highest_costs) {
      highest_costs_id = c;
      highest_costs = costs->array[c].unit_costs;
    }
  }

  // Prepare results
  double result = highest_costs;

  // Iterate through dependencies
  for (int d=0; d<5; d++) {
    if (costs->array[highest_costs_id].dependencies[d] != -1) {
      result += getUnitCosts(costs->array[highest_costs_id].dependencies[d], costs);
    }
  }

  return result;

}

int readDivisionInformation() {

  // Make dynamic array for cost information
  division_unit_costs_array costs;
  init_division_unit_costs_array(&costs);

  // Begin iteration
  RUNNING_COSTS = 0;
  START_DIVISION_INFORMATION_MESSAGE_LOOP

    // Check if relevant
    if (division_information_message->firm_id == ID) {

      // Store cost information
      RUNNING_COSTS += division_information_message->running_costs;
      add_division_unit_costs(&costs,
        division_information_message->id,
        division_information_message->good_type,
        division_information_message->unit_costs,
        division_information_message->dependencies);

    }

  FINISH_DIVISION_INFORMATION_MESSAGE_LOOP

  for (int p=0; p<PRODUCTS.size; p++) {
    PRODUCTS.array[p].unit_production_cost = getUnitCosts(PRODUCTS.array[p].good_type, &costs);
  }

  // Clean up
  free_division_unit_costs_array(&costs);

  return 0;

}
