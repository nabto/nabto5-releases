# Changelog

## [5.9.0] - 2022-05-31

### Changed
 * On macOS and iOS, a system mDNS implementation is now used instead of a custom implementation. All APIs and semantics are the same - but it is now simpler to pass Apple's reviews.

### Bugfixes
 * Fixed a bug in client relay endpoint shuffle.

## [5.8.0] - 2021-11-15

### Bugfixes
 * There was a buffer read overrun in the stun client.

### Added
 * nabto_client_stop2 and nabto_client_future_set_callback2 which provides an
   error code if the underlying operation would give an error

### Changed
 * The documentation for stop behavior has been updated. Some undocumented
   behavior has been documented and in the process the code has been updated to
   match the documentation.
 * nabto_client_stream_abort has changed name to nabto_client_stream_stop and
   the former has been deprecated.
 * NABTO_CLIENT_EC_ABORTED has been made an alias for NABTO_CLIENT_EC_STOPPED
   and they now has the same value. NABTO_CLIENT_EC_ABORTED has been deprecated.
 * Versioning of master and feature branches has changed.

## [5.7.0] - 2021-09-21

### Added
 * Connection option to control timeout of first DTLS response from device (milliseconds): `DtlsHelloTimeout`. Note that the default value is 10 seconds which then replaces the earlier fixed timeout of 120 seconds.

### Changed
 * Only relevant symbols are exported allowing Nabto 5/Edge client to coexist with Nabto 4/Micro clients.
 * Several minor bug fixes and leaks


## [5.4.1] - 2021-06-02

### Changed
  * Add armv7s support on iOS and change build to use native cmake iOS support.

## [5.4.0] - 2021-02-23

### Changed
  * Mdns uses multiple interfaces when querying for local devices. Previously only the OS configured default interface was used.

## [5.2.0] - 2020-11-23

### Added

  * Password authentication using PAKE.
  * MDNS subtype resolving
  * MDNS txt records resolving.
  * Stable MDNS scan interface.
  * Stop functions for several types to help RAII usage patterns.


### Changed

  * Direct ip connections can now be made without having a device and product id.

### Breaking Changes

#### MDNS subtype connections

Before 5.2 the default mdns connection behavior was to scan for all
devices and search the the device in the list of all the
devices. After 5.2 including 5.2 the default behavior is to use the
mdns subtype of the device when creating a local connection.  To use
local connections with devices which is older than 5.2 the option
ScanLocalConnect needs to be set.

#### Local UDP Ping when using direct candidates.

After 5.2 including 5.2 the clients is doing a local udp ping before a
direct candidate connection is made. Devices which is older than 5.2
does not support local ping and hence does not work with direct
candidates.

## [5.1.0] - 2020-06-30

## Added

 - Server Connect Tokens.
 - Tunnel support is moved from experimental to stable.

## [5.0.0] - 2019-12-01

Initial release.
