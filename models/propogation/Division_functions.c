#include <math.h>

#include "header.h"
#include "Division_agent_header.h"
#include "Division_helpers.h"

int produce() {

  // Calculate the potential output of physical and human capital
  OUTPUT_INFORMATION.physical_capital_potential = PHYSICAL_CAPITAL.quantity * PHYSICAL_CAPITAL.production_per_unit;
  OUTPUT_INFORMATION.human_capital_potential = HUMAN_CAPITAL.quantity * HUMAN_CAPITAL.production_per_unit;

  // Calculate the overall potential output of available output
  int capital_potential =
    (OUTPUT_INFORMATION.physical_capital_potential < OUTPUT_INFORMATION.human_capital_potential) ?
    OUTPUT_INFORMATION.physical_capital_potential : OUTPUT_INFORMATION.human_capital_potential;

  // Prepare for iteration to find the current potential of the production inventory
  OUTPUT_INFORMATION.production_inventory_potential = 0;
  int current_production_inventory_potential;

  // For iteration
  int p;

  // Iterate through production inventory components
  for (p=0; p<PRODUCTION_INVENTORY.size; p++) {

    // Calculate the potential output of this component
    current_production_inventory_potential = PRODUCTION_INVENTORY.array[p].current /
      PRODUCTION_INVENTORY.array[p].requirement_per_unit_output;

    // Check if the overall potential output of the inventory should be updated
    if (p == 0 || current_production_inventory_potential < OUTPUT_INFORMATION.production_inventory_potential) {
      OUTPUT_INFORMATION.production_inventory_potential = current_production_inventory_potential;
    }

  }

  // Compute the output potential of available funding
  OUTPUT_INFORMATION.funding_potential = floor(FUNDING.production / PRODUCTION_COSTS_PER_UNIT);

  // Calculate the overall output potential of the production inventory and available funding
  int provisions_potential =
    (OUTPUT_INFORMATION.production_inventory_potential < OUTPUT_INFORMATION.funding_potential) ?
    OUTPUT_INFORMATION.production_inventory_potential : OUTPUT_INFORMATION.funding_potential;

  // Calculate overall output potential
  OUTPUT_INFORMATION.potential = (capital_potential < provisions_potential) ?
    capital_potential : provisions_potential;

  // Compare output potential to stock requirement to decide on an actual production amount
  int output_inventory_requirement = OUTPUT_INVENTORY.target - OUTPUT_INVENTORY.current;
  OUTPUT_INFORMATION.actual = output_inventory_requirement < OUTPUT_INFORMATION.potential ?
    output_inventory_requirement : OUTPUT_INFORMATION.potential;

  // Update the inventories and funding based on production
  OUTPUT_INVENTORY.current += OUTPUT_INFORMATION.actual;
  for (p=0; p<PRODUCTION_INVENTORY.size; p++) {
    PRODUCTION_INVENTORY.array[p].current -=
      PRODUCTION_INVENTORY.array[p].requirement_per_unit_output * OUTPUT_INFORMATION.actual;
  }
  FUNDING.production -= OUTPUT_INFORMATION.actual * PRODUCTION_COSTS_PER_UNIT;

  return 0;

}

int stateOutputAvailability() {

  // State availability of output if any exists
  if (OUTPUT_INVENTORY.current > 0) {
    add_division_output_availability_message(FIRM_ID, ID, OUTPUT_GOOD_ID,
      COSTS.unit, OUTPUT_INVENTORY.current);
  }

  return 0;

}

int checkProductionInventory() {

  // Variable initialisation
  int restock_amount, order_amount, p, r;

  // Initialise storage of relevant availabilities
  division_availability_information_array relevant_availabilities;
  init_division_availability_information_array(&relevant_availabilities);

  START_DIVISION_OUTPUT_AVAILABILITY_MESSAGE_LOOP

    // Check the availability information is relevant
    if (division_output_availability_message->firm_id == FIRM_ID &&
    division_output_availability_message->id != ID) {

      // Store information about resource availability
      add_division_availability_information(&relevant_availabilities,
        division_output_availability_message->id,
        division_output_availability_message->good_id,
        division_output_availability_message->unit_cost,
        division_output_availability_message->quantity);

    }

  FINISH_DIVISION_OUTPUT_AVAILABILITY_MESSAGE_LOOP

  // Sort relevant availabilities by unit costs (to get cheapest first)
  qsort(relevant_availabilities.array, relevant_availabilities.size,
    sizeof(relevant_availabilities), compare_availability_costs);

  for (p=0; p<PRODUCTION_INVENTORY.size; p++) {

    // Determine the amount to restock
    restock_amount = (PRODUCTION_INVENTORY.array[p].target -
      (PRODUCTION_INVENTORY.array[p].current + PRODUCTION_INVENTORY.array[p].ordered));

    for (r=0; r<relevant_availabilities.size; r++) {

      // Check if further orders are required
      if (restock_amount <= 0) {
        break;
      }

      // Check that both the production inventory component and availability concern the same good
      else if (PRODUCTION_INVENTORY.array[p].good_id == relevant_availabilities.array[r].good_id) {

        // Determine the amount to order and adjust amount remaining to order
        order_amount = restock_amount < relevant_availabilities.array[r].quantity ?
          restock_amount : relevant_availabilities.array[r].quantity;
        restock_amount -= order_amount;

        // Send order message
        // add_division_order_message(FIRM_ID, ID, relevant_availabilities.array[r].id, order_amount);

      }

    }

  }

  // Clean up availability storage
  free_division_availability_information_array(&relevant_availabilities);

  return 0;

}

int processOrders() {

  int accepted_amount;

  START_DIVISION_ORDER_MESSAGE_LOOP

    // Check that there is remaining stock and that the order is relevant
    if (OUTPUT_INVENTORY.current > 0 && division_order_message->firm_id == FIRM_ID &&
    division_order_message->seller_id == ID) {

      // Calculate amount to accept, adjust output inventory accordingly
      accepted_amount = OUTPUT_INVENTORY.current < division_order_message->quantity ?
        OUTPUT_INVENTORY.current : division_order_message->quantity;
      OUTPUT_INVENTORY.current -= accepted_amount;

      // Send confirmation message
      add_division_order_confirmation_message(FIRM_ID, ID,
        division_order_message->buyer_id, accepted_amount);

    }

  FINISH_DIVISION_ORDER_MESSAGE_LOOP

  return 0;

}
