#include <math.h>
#include "Division_helpers.h"

division_transport_quote calculate_transport_costs(
  division_transport_information * source_information,
  double destination_x, double destination_y, double quantity)
{

  int vehicles_required;
  double distance, delivery_time, transport_costs;

  // Calculate the distance travelled, and from it the delivery time
  distance = sqrt(
    pow((source_information->x_position - destination_x), 2) +
    pow((source_information->y_position - destination_y), 2) );
  delivery_time = distance / source_information->vehicle_speed;

  // Calculate the number of vehicles required, and the transport costs
  vehicles_required = quantity / source_information->vehicle_capacity;
  transport_costs = (
    (((double) vehicles_required) * source_information->vehicle_cost_per_day) +
    (((double) quantity) * source_information->unit_cost_per_day));

  // Build a result object
  division_transport_quote result;
  init_division_transport_quote(&result);
  result.cost = transport_costs;
  result.time = floor(delivery_time);

  // Return the result object
  return result;

}
