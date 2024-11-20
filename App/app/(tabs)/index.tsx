import React from "react";
import { createStackNavigator } from "@react-navigation/stack";
import TabOneScreen from "../Pages/SelectDeviceScreen";
import ControlDeviceScreen from "../Pages/ControlDeviceScreen";

export type RootStackParamList = {
  "Select Device": undefined;
  "Control Device": { device: { name: string; id: string } };
};

const Stack = createStackNavigator<RootStackParamList>();

export default function AppNavigator() {
  return (
    <Stack.Navigator>
      <Stack.Screen name="Select Device" component={TabOneScreen} />
      <Stack.Screen name="Control Device" component={ControlDeviceScreen} />
    </Stack.Navigator>
  );
}
