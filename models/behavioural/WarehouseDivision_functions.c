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
