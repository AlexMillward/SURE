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

int stateProductionInventoryRequirements() {

  double required_amount;

  for (int p=0; p<PRODUCTION_INVENTORY.size; p++) {

    // Check if a restock is required
    if ((PRODUCTION_INVENTORY.array[p].current + PRODUCTION_INVENTORY.array[p].ordered) <
      PRODUCTION_INVENTORY.array[p].restock_level)
    {

      // Calculate the required amount
      required_amount = PRODUCTION_INVENTORY.array[p].target -
        (PRODUCTION_INVENTORY.array[p].current + PRODUCTION_INVENTORY.array[p].ordered);

      // Add the requirement message
      add_division_requirement_message(FIRM_ID, ID,
        PRODUCTION_INVENTORY.array[p].good_id, required_amount,
        TRANSPORT_INFORMATION.x_position, TRANSPORT_INFORMATION.y_position);

    }

  }

  return 0;

}


int offerOutput() {

  int offered = 0;

  START_DIVISION_REQUIREMENT_MESSAGE_LOOP

    // Check the message is relevant and that the quantity is available
    if (FIRM_ID == division_requirement_message->firm_id &&
      OUTPUT_GOOD_ID == division_requirement_message->good_id &&
      (OUTPUT_INVENTORY.current - offered) >= division_requirement_message->quantity)
    {

      // Adjust the amount offered
      offered += division_requirement_message->quantity;

      // Calculate transport costs
      division_transport_quote transport_quote = calculate_transport_costs(
        &TRANSPORT_INFORMATION,
        division_requirement_message->x_position,
        division_requirement_message->y_position,
        division_requirement_message->quantity);

      // Add the offer message
      add_division_offer_message(FIRM_ID, ID,
        division_requirement_message->id,
        division_requirement_message->good_id,
        division_requirement_message->quantity,
        UNIT_COST + transport_quote.cost, transport_quote.time);

      // Clean up
      free_division_transport_quote(&transport_quote);

    }

  FINISH_DIVISION_REQUIREMENT_MESSAGE_LOOP

  return 0;

}

int processOffers() {

  // Initialise required variables
  int current_id, b;
  division_offer_information_array best_offers;
  init_division_offer_information_array(&best_offers);

  START_DIVISION_OFFER_MESSAGE_LOOP

    // Check relevance of message
    if (FIRM_ID == division_offer_message->firm_id && ID == division_offer_message->target_id) {

      // Find entry for good considered if one exists
      current_id = -1;
      for (b=0; b<best_offers.size; b++) {
        if (best_offers.array[b].good_id == division_offer_message->good_id) {
          current_id = b;
          break;
        }
      }

      // Good considered not previously considered
      if (current_id == -1) {
        add_division_offer_information(&best_offers,
          division_offer_message->source_id,
          division_offer_message->quantity,
          division_offer_message->cost,
          division_offer_message->delivery_time);
      }

      // Good considered previously considered, but has a lower cost
      else if (division_offer_message->cost < best_offers.array[b].cost) {
        best_offers.array[b].source_id = division_offer_message->source_id;
        best_offers.array[b].quantity = division_offer_message->quantity;
        best_offers.array[b].cost = division_offer_message->cost;
        best_offers.array[b].delivery_time = division_offer_message->delivery_time;
      }

    }

  FINISH_DIVISION_OFFER_MESSAGE_LOOP

  // TODO : Process best offers

  // Clean up
  free_division_offer_information_array(&best_offers);

  return 0;

}
