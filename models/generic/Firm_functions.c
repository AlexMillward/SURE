#include <stdlib.h>

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

/*
int allocateRunningFunds() {

  START_DIVISION_INFORMATION_MESSAGE_LOOP

    // Check division belongs to firm
    if (division_information_message->firm_id == ID) {

      // Allocate funds
      FUNDS -= division_information_message->running_costs;
      add_funds_allocation_message(
        division_information_message->id,
        division_information_message->running_costs);

    }

  FINISH_DIVISION_INFORMATION_MESSAGE_LOOP

  return 0;

}
*/

int decidePrices() {

  // Compute the proportion of sales expectation for each product
  int p;
  int sales_expectation_total = 0;

  for (p=0; p<PRODUCTS.size; p++) {

    sales_expectation_total += PRODUCTS.array[p].sales_expectation;

  }

  // Compute the weightings for each product (overall, including sales proportion)
  double weightings[PRODUCTS.size];
  double weightings_total = 0;

  for (p=0; p<PRODUCTS.size; p++) {

    weightings[p] = (((double) PRODUCTS.array[p].sales_expectation) /
      ((double) sales_expectation_total)) * PRODUCTS.array[p].running_costs_contribution;
    weightings_total += weightings[p];

  }

  // Determine the price for each product (unit costs + running costs allocation)
  double allocation, unit_allocation;

  for (p=0; p<PRODUCTS.size; p++) {

    allocation = (weightings[p] / weightings_total) * RUNNING_COSTS;
    unit_allocation = allocation / PRODUCTS.array[p].sales_expectation;
    PRODUCTS.array[p].price = PRODUCTS.array[p].unit_production_cost + unit_allocation;

  }

  return 0;

}
