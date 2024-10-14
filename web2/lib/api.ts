export const BASE_URL = "/";
import { createApiClient } from "@/api/api-zod";

export const api = createApiClient(BASE_URL);
