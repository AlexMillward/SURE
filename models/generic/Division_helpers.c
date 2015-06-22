#include "Division_helpers.h"

double computeUnitCosts(int current, division_cost_information_array * costs) {

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
    if (costs->array[c].good_type == current && costs->array[c].production_costs_per_unit > highest_costs) {
      highest_costs_id = c;
      highest_costs = costs->array[c].production_costs_per_unit;
    }
  }

  // Prepare results
  double result = highest_costs;

  // Iterate through dependencies
  for (int d=0; d<5; d++) {
    if (costs->array[highest_costs_id].dependencies[d] != -1) {
      result += computeUnitCosts(costs->array[highest_costs_id].dependencies[d], costs);
    }
  }

  return result;

}
