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
  int product_number, p, division_number;
  double price;

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

      // Calculate price
      price = cost_information.unit_cost + cost_information.running_costs_per_unit +
        cost_information.profit_estimate

      // Store division information
      division_number = DIVISION_DETAILS.size;
      add_firm_division_details(&DIVISION_DETAILS,
        division_availability_message->division_id,
        division_availability_message->quantity,
        price,
        &cost_information);

      // Broadcast update (note that cost information is meant to be hidden from the market, is provided for ease of implementation)
      add_firm_availability_message(ID,
        division_number,
        division_availability_message->good_id,
        division_availability_message->quantity,
        price,
        division_availability_message->transport_information);

    }

  FINISH_DIVISION_AVAILABILITY_MESSAGE_LOOP

  return 0;

}

int processOrders() {

  firm_division_details * current_division_details;

  START_PRODUCT_ORDER_MESSAGE_LOOP

    current_division_details = DIVISION_DETAILS.array[product_order_message->division_number];

    if (product_order_message->quantity < current_division_details.quantity) {

      // Adjust firm funds
      CURRENT_FUNDS += (current_division_details->price - current_division_details->unit_cost) *
        product_order_message->quantity;

      // Instruct division to adjust funds and stock
      add_firm_instruction_message(ID, current_division_details->id,
        product_order_message->quantity,
        product_order_message->quantity * current_division_details->unit_cost);

      // Provide order confirmation to the retailer

    }

  FINISH_PRODUCT_ORDER_MESSAGE_LOOP

  return 0;

}
