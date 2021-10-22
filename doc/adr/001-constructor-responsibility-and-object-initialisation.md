# 1. Constructor responsibility and object initialisation

Date: 2021-05-07

## Status

Accepted

## Context

Static memory allocation (i.e. global variables/objects) is made before `setup()` execution - before any initialisation can be made (i.e. serial communication).

## Decision

All constructors for global objects should not make any initialisation or validation that may result in some form of communication - especialy `Serial.print()`.
This functionality must me executed in `setup()`.

## Consequences
Object can have invalid state between instantiation and execution of `initialise()`.
