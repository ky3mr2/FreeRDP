/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * Popup browser for AAD authentication
 *
 * Copyright 2023 Isaac Klein <fifthdegree@protonmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *		 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string>
#include <sstream>
#include <cstdlib>
#include <memory>
#include <array>
#include <winpr/string.h>
#include <freerdp/log.h>
#include <freerdp/utils/aad.h>
#include <freerdp/build-config.h>

#include "sdl_webview.hpp"
#include "webview_impl.hpp"

#define TAG CLIENT_TAG("SDL.webview")

static std::string from_settings(const rdpSettings* settings, FreeRDP_Settings_Keys_String id)
{
	auto val = freerdp_settings_get_string(settings, id);
	if (!val)
	{
		WLog_WARN(TAG, "Settings key %s is NULL", freerdp_settings_get_name_for_key(id));
		return "";
	}
	return val;
}

static std::string from_aad_wellknown(rdpContext* context, AAD_WELLKNOWN_VALUES which)
{
	auto val = freerdp_utils_aad_get_wellknown_string(context, which);

	if (!val)
	{
		WLog_WARN(TAG, "[wellknown] key %s is NULL", freerdp_utils_aad_wellknwon_value_name(which));
		return "";
	}
	return val;
}

std::string webview_exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

static BOOL sdl_webview_get_rdsaad_access_token(freerdp* instance, const char* scope,
                                                const char* req_cnf, char** token)
{
	WINPR_ASSERT(instance);
	WINPR_ASSERT(scope);
	WINPR_ASSERT(req_cnf);
	WINPR_ASSERT(token);

	WINPR_UNUSED(instance);
	WINPR_UNUSED(instance->context);

	auto client_id = from_settings(instance->context->settings, FreeRDP_GatewayAvdClientID);
	std::string redirect_uri = "ms-appx-web%3a%2f%2fMicrosoft.AAD.BrokerPlugin%2f" + client_id;

	*token = nullptr;

	auto ep = from_aad_wellknown(instance->context, AAD_WELLKNOWN_authorization_endpoint);
	auto url = ep + "?client_id=" + client_id + "&response_type=code&scope=" + scope +
	           "&redirect_uri=" + redirect_uri;

	const std::string title = "FreeRDP WebView - AAD access token";

	std::string code;
	
	//auto rc = webview_impl_run(title, url, code);
	std::string installpath = FREERDP_INSTALL_PREFIX;
	std::string command = installpath + "/bin/webview_window \"" + title + "\" \"" + url + "\"";
    code = webview_exec(command.c_str());


	if (code.empty())
		return FALSE;

	auto token_request = "grant_type=authorization_code&code=" + code + "&client_id=" + client_id +
	                     "&scope=" + scope + "&redirect_uri=" + redirect_uri +
	                     "&req_cnf=" + req_cnf;
	return client_common_get_access_token(instance, token_request.c_str(), token);
}

static BOOL sdl_webview_get_avd_access_token(freerdp* instance, char** token)
{
	WINPR_ASSERT(token);
	WINPR_ASSERT(instance);
	WINPR_ASSERT(instance->context);

	auto client_id = from_settings(instance->context->settings, FreeRDP_GatewayAvdClientID);
	std::string redirect_uri = "ms-appx-web%3a%2f%2fMicrosoft.AAD.BrokerPlugin%2f" + client_id;
	std::string scope = "https%3A%2F%2Fwww.wvd.microsoft.com%2F.default";

	*token = nullptr;

	auto ep = from_aad_wellknown(instance->context, AAD_WELLKNOWN_authorization_endpoint);
	auto url = ep + "?client_id=" + client_id + "&response_type=code&scope=" + scope +
	           "&redirect_uri=" + redirect_uri;
	const std::string title = "FreeRDP WebView - AVD access token";
	std::string code;
	auto rc = webview_impl_run(title, url, code);
	if (!rc || code.empty())
		return FALSE;

	auto token_request = "grant_type=authorization_code&code=" + code + "&client_id=" + client_id +
	                     "&scope=" + scope + "&redirect_uri=" + redirect_uri;
	return client_common_get_access_token(instance, token_request.c_str(), token);
}

BOOL sdl_webview_get_access_token(freerdp* instance, AccessTokenType tokenType, char** token,
                                  size_t count, ...)
{
	WINPR_ASSERT(instance);
	WINPR_ASSERT(token);
	switch (tokenType)
	{
		case ACCESS_TOKEN_TYPE_AAD:
		{
			if (count < 2)
			{
				WLog_ERR(TAG,
				         "ACCESS_TOKEN_TYPE_AAD expected 2 additional arguments, but got %" PRIuz
				         ", aborting",
				         count);
				return FALSE;
			}
			else if (count > 2)
				WLog_WARN(TAG,
				          "ACCESS_TOKEN_TYPE_AAD expected 2 additional arguments, but got %" PRIuz
				          ", ignoring",
				          count);
			va_list ap = {};
			va_start(ap, count);
			const char* scope = va_arg(ap, const char*);
			const char* req_cnf = va_arg(ap, const char*);
			const BOOL rc = sdl_webview_get_rdsaad_access_token(instance, scope, req_cnf, token);
			va_end(ap);
			return rc;
		}
		case ACCESS_TOKEN_TYPE_AVD:
			if (count != 0)
				WLog_WARN(TAG,
				          "ACCESS_TOKEN_TYPE_AVD expected 0 additional arguments, but got %" PRIuz
				          ", ignoring",
				          count);
			return sdl_webview_get_avd_access_token(instance, token);
		default:
			WLog_ERR(TAG, "Unexpected value for AccessTokenType [%" PRIuz "], aborting", tokenType);
			return FALSE;
	}
}
