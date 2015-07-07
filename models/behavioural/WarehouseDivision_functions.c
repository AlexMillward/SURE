#include "header.h"
#include "WarehouseDivision_agent_header.h"

int WD_report() {

  // Report costs
  add_division_administration_report_message(FIRM_ID, DIVISION_ID,
    -1, CUMULATIVE_COSTS, FUNDS);

  // Reset costs
  FUNDS = 0;
  CUMULATIVE_COSTS = 0;

  // Report inventory statistics
  add_division_inventory_report_message(FIRM_ID, DIVISION_ID,
    CUMULATIVE_RECIEVED, CUMULATIVE_DISPATCHED,
    CUMULATIVE_DEFICIT, CURRENT_INVENTORY);

  // Reset inventory statistics
  CUMULATIVE_RECIEVED = 0;
  CUMULATIVE_DISPATCHED = 0;
  CUMULATIVE_DEFICIT = 0;

  return 0;

}

int WD_marketing_report() {

  // Send report of inventory to the marketing division
  add_division_marketing_inventory_report_message(FIRM_ID, CUMULATIVE_DISPATCHED);

  return 0;

}

int WD_update_deliveries() {

  int d=0;
  while (d<DELIVERIES.size) {

    // Reduce counter until arrival
    DELIVERIES.array[d].time--;

    // Check if the delivery has arrived
    if (DELIVERIES.array[d].time <= 0) {

      // Update the inventory
      CURRENT_INVENTORY += DELIVERIES.array[d].quantity;

      // Remove the delivery from tracking
      remove_single_type_record(&DELIVERIES, d);

    } else {
      d++;
    }

  }

  return 0;

}

int WD_add_delivery() {

  // Add deliveries
  START_DIVISION_DELIVERY_NOTIFICATION_MESSAGE_LOOP
    add_single_type_record(&DELIVERIES,
      division_delivery_notification_message->quantity,
      division_delivery_notification_message->delivery_time);
  FINISH_DIVISION_DELIVERY_NOTIFICATION_MESSAGE_LOOP

  return 0;

}
