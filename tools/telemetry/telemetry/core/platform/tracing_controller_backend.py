# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from telemetry.core.platform import tracing_category_filter
from telemetry.core.platform import tracing_options
from telemetry.timeline import trace_data as trace_data_module


class TracingControllerBackend(object):
  def __init__(self, platform_backend):
    self._platform_backend = platform_backend
    self._current_trace_options = None
    self._current_category_filter = None

  def Start(self, trace_options, category_filter, timeout):
    if self.is_tracing_running:
      return False

    assert isinstance(category_filter,
                      tracing_category_filter.TracingCategoryFilter)
    assert isinstance(trace_options,
                      tracing_options.TracingOptions)

    self._AssertOneBrowserBackend()

    self._current_trace_options = trace_options
    self._current_category_filter = category_filter

    if trace_options.enable_chrome_trace:
      browser_backend = self.running_browser_backends[0]
      browser_backend.StartTracing(
          trace_options, category_filter.filter_string, timeout)

    if trace_options.enable_platform_display_trace:
      self._platform_backend.StartDisplayTracing()

  def Stop(self):
    assert self.is_tracing_running, 'Can only stop tracing when tracing.'
    self._AssertOneBrowserBackend()

    trace_data_builder = trace_data_module.TraceDataBuilder()
    if self._current_trace_options.enable_chrome_trace:
      browser_backend = self.running_browser_backends[0]
      browser_backend.StopTracing(trace_data_builder)

    if self._current_trace_options.enable_platform_display_trace:
      surface_flinger_trace_data = self._platform_backend.StopDisplayTracing()
      trace_data_builder.AddEventsTo(
          trace_data_module.SURFACE_FLINGER_PART, surface_flinger_trace_data)

    self._current_trace_options = None
    self._current_category_filter = None
    return trace_data_builder.AsData()

  def _AssertOneBrowserBackend(self):
    # Note: it is possible to implement tracing for both the case of 0 and >1.
    # For >1, we just need to merge the trace files at StopTracing.
    #
    # For 0, we want to modify chrome's trace-startup to support leaving
    # tracing on indefinitely. Then have the backend notify the platform
    # and the tracing controller that it is starting a browser, have
    # the controller add in the trace-startup command, and then when we get
    # the Stop message or the DidStopBrowser(), issue the stop tracing command
    # on the right backend.
    num_instances = len(self.running_browser_backends)
    assert num_instances == 1, (
        'Tracing only supports one browser instance (not %i).' % num_instances)

  def IsChromeTracingSupported(self):
    self._AssertOneBrowserBackend()
    browser_backend = self.running_browser_backends[0]
    return browser_backend.devtools_client.IsChromeTracingSupported()

  def IsDisplayTracingSupported(self):
    return self._platform_backend.IsDisplayTracingSupported()

  @property
  def is_tracing_running(self):
    return self._current_trace_options != None

  @property
  def running_browser_backends(self):
    return self._platform_backend.running_browser_backends

  def DidStartBrowser(self, browser, browser_backend):
    pass

  def WillCloseBrowser(self, browser, browser_backend):
    pass
