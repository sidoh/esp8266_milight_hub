import { AliasesApi, DeviceControlApi, DeviceControlApiFp } from "@/api";

export const BASE_URL = "";
import { Configuration } from "@/api";

const config = new Configuration({ basePath: BASE_URL });

export const api = {
    deviceControl: new DeviceControlApi(config),
    aliases: new AliasesApi(config),
}