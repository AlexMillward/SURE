#include <stdlib.h>

#include "header.h"
#include "Firm_agent_header.h"

int readDivisionUnitCosts() {

  int p;

  for (p=0; p<PRODUCTS>size; p++) {

    PRODUCTS.array[p].unit_production_cost = -1;

  }

  START_DIVISION_UNIT_COSTS_MESSAGE_LOOP

    // Determine if relevant
    if (division_unit_costs_message->firm_id == ID) {

      for (p=0; p<PRODUCTS.size; p++) {

        if (PRODUCTS.array[p].good_type == division_unit_costs_message->good_type &&
            division_unit_costs_message->unit_costs > PRODUCTS.array[p].unit_production_cost) {
          PRODUCTS.array[p].unit_production_cost = ;
        }

      }

    }

  FINISH_DIVISION_UNIT_COSTS_MESSAGE_LOOP

  return 0;

}

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
