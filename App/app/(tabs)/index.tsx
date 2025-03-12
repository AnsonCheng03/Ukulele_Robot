import { createStackNavigator } from "@react-navigation/stack";
import SelectDeviceScreen from "../Pages/SelectDeviceScreen";
import ControlDeviceScreen from "../Pages/ControlDeviceScreen";

export type RootStackParamList = {
  "Select Device": undefined;
  "Control Device": {
    device: {
      connect(): unknown;
      discoverAllServicesAndCharacteristics(): unknown;
      name: string;
      id: string;
    };
  };
};

const Stack = createStackNavigator<RootStackParamList>();

export default function HomeScreen() {
  return (
    <Stack.Navigator>
      <Stack.Screen name="Select Device" component={SelectDeviceScreen} />
      <Stack.Screen name="Control Device" component={ControlDeviceScreen} />
    </Stack.Navigator>
  );
}
