#include "header.h"
#include "Firm_agent_header.h"

int calculateCosts() {

  RUNNING_COSTS = 0;

  // Find the running costs total
  START_DIVISION_COSTS_MESSAGE_LOOP

    RUNNING_COSTS += division_costs_message->running_costs;

  FINISH_DIVISION_COSTS_MESSAGE_LOOP

  int p;
  double allocation_total = 0;

  // Find the total of allocation numbers, can be ensured that actual values sum to 1
  for (p=0; p<PRODUCTS.size; p++) {
    allocation_total += PRODUCTS.array[p].running_costs_allocation;
  }

  // Determine running cost allocation to each unit that will be sold
  for (p=0; p<PRODUCTS.size; p++) {
    PRODUCTS.array[p].running_costs_per_unit =
      ((PRODUCTS.array[p].running_costs_allocation / allocation_total) *
      RUNNING_COSTS) / ((double) PRODUCTS.array[p].sales.expectation);
  }

  return 0;

}

int processOutputAvailability() {

  // Variable initialisation
  int product_number, p;

  START_DIVISION_AVAILABILITY_MESSAGE_LOOP

    // Find product for the good type
    product_number = -1;
    for (p=0; p<PRODUCTS.size; p++) {
      if (PRODUCTS.array[p].good_id == division_availability_message->good_id) {
        product_number = p;
      }
    }

    // Check the good type should be sold on the market
    if (product_number != -1) {

      // Compute different cost information
      firm_cost_information cost_information;
      init_firm_cost_information(&cost_information);
      cost_information.unit_cost = division_availability_message->unit_cost;
      cost_information.running_costs_per_unit = PRODUCTS.array[p].running_costs_per_unit;
      cost_information.profit_estimate = ((100 + PRODUCTS.array[p].percentage_profit_target) / 100) *
        cost_information.unit_cost * cost_information.running_costs_per_unit;

      // Broadcast update (note that cost information is meant to be hidden from the market, is provided for ease of implementation)
      add_firm_availability_message(ID,
        division_availability_message->division_id,
        division_availability_message->good_id,
        division_availability_message->quantity,
        cost_information,
        cost_information.unit_cost +
          cost_information.running_costs_per_unit +
          cost_information.profit_estimate,
        division_availability_message->transport_information);

    }

  FINISH_DIVISION_AVAILABILITY_MESSAGE_LOOP

  return 0;

}

int processOrders() {

  return 0;

}
