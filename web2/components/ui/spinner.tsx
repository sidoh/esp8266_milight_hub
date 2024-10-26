import * as React from "react";
import { Loader } from "lucide-react";

export const Spinner: React.FC<{ className?: string }> = ({ className }) => (
  <Loader size={16} className={`animate-spin ${className}`} />
);
