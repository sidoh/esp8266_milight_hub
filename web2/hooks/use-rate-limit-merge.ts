import { useState, useEffect, useRef } from "react";

interface StateWithSerial<T> {
  value: T;
  serial: number;
}

export function useRateLimitMerge<T>(
  initialValue: T,
  delay: number
): [StateWithSerial<T>, (newValue: T | ((prevValue: T) => T)) => void, () => void] {
  const [state, setState] = useState<StateWithSerial<T>>({
    value: initialValue,
    serial: 0,
  });
  const timeoutRef = useRef<NodeJS.Timeout | null>(null);
  const lastCallTimeRef = useRef<number>(0);

  useEffect(() => {
    return () => {
      if (timeoutRef.current) {
        clearTimeout(timeoutRef.current);
      }
    };
  }, []);

  const rateLimitedSetValue = (newValue: T | ((prevValue: T) => T)) => {
    const now = Date.now();
    const timeSinceLastCall = now - lastCallTimeRef.current;

    const updateState = (prevState: StateWithSerial<T>) => {
      lastCallTimeRef.current = now;
      const updatedValue =
        typeof newValue === "function"
          ? (newValue as (prevValue: T) => T)(prevState.value)
          : newValue;
      const mergedValue =
        typeof prevState.value === "object" && typeof updatedValue === "object"
          ? { ...prevState.value, ...updatedValue }
          : updatedValue;
      return {
        value: mergedValue,
        serial: prevState.serial + 1,
      };
    };

    if (timeSinceLastCall >= delay) {
      // If enough time has passed, update immediately
      setState(updateState);
    } else {
      // Otherwise, schedule an update
      if (timeoutRef.current) {
        clearTimeout(timeoutRef.current);
      }
      timeoutRef.current = setTimeout(() => {
        setState(updateState);
      }, delay - timeSinceLastCall);
    }
  };

  const clearValue = () => {
    setState({ value: initialValue, serial: 0 });
    lastCallTimeRef.current = 0;
    if (timeoutRef.current) {
      clearTimeout(timeoutRef.current);
    }
  };

  return [state, rateLimitedSetValue, clearValue];
}
