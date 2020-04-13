from pyenergyplus.plugin import EnergyPlusPlugin


class RH_OpeningController(EnergyPlusPlugin):

    def __init__(self):
        super().__init__()
        self.need_to_get_handles = True
        self.ZoneRH_handle = None
        self.MyOpenFactor_handle = None

    def on_begin_timestep_before_predictor(self) -> int:
        if self.need_to_get_handles and self.api.exchange.api_data_fully_ready():
            self.ZoneRH_handle = self.api.exchange.get_variable_handle(
                "System Node Relative Humidity",
                "Zone 1 Node"
            )

            self.MyOpenFactor_handle = self.api.exchange.get_actuator_handle(
                "AirFlow Network Window/Door Opening",
                "Venting Opening Factor",
                "Zn001:Wall001:Win001"
            )

            self.need_to_get_handles = False
        else:
            return 0

        zone_rh = self.api.exchange.get_variable_value(self.ZoneRH_handle)

        if zone_rh < 25.0:
            self.api.exchange.set_actuator_value(self.MyOpenFactor_handle, 0.0)
        elif zone_rh > 60.0:
            self.api.exchange.set_actuator_value(self.MyOpenFactor_handle, 1.0)
        else:
            self.api.exchange.set_actuator_value(self.MyOpenFactor_handle, (zone_rh - 25) / (60 - 25))

        return 0
