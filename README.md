RaspSurveillance
================

A Surveillance Station using (multiple) webcam(s) and a Raspberry Pi


Requirements
============

* Opencv
* Sqlite
* Curl
* Openssl
* A configured MTA (for example msmtp-mta)


Config
======
RaspSurveillance looks for the configuration file in the following order:
``$XDG_CONFIG_HOME/rpisecsys/config``, ``~/.config/rpisecsys/config``,
	``$XDG_CONFIG_DIRS/rpisecsys/config``, ``/etc/xdg/rpisecsys.cfg``.

RaspSurveillance has two main simple configuration options under [options]
* database, the default sqlite3 database location.
* image_directory, the location where the motion detected images are saved.

Then there are currently two notification options for RaspSurveillance, NotifyMyAndroid and E-mail.

NotifyMyAndroid
---------------
Just fill in your api key under [nma] in api_key

E-mail
------
E-mail requires a setup of a MTA, like esmtp or msmtp-mta.

Usage
=====
Just run the `watcher` program.
