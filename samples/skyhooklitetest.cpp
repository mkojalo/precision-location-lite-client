/**
 * \author Skyhook Wireless
 *
 * \section license LIMITED USE LICENSE
 *
 * Copyright 2014-present Skyhook Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <skyhookliteclient.h>
#include <stdio.h>

static void
print_location(const SHLC_Location* location)
{
    printf("%f, %f\t+/-%.0fm\t%d+%d+%d  %lums\n",
           location->latitude,
           location->longitude,
           location->hpe,
           location->nap,
           location->ncell,
           location->nsat,
           location->age);

    if (location->speed >= 0)
    {
        printf("\t%.1fkm/h", location->speed);

        if (location->bearing >= 0)
            printf("\t%.0f", location->bearing);

        printf("\n");
    }
}

/*********************************************************************/
/*                                                                   */
/* main                                                              */
/*                                                                   */
/*********************************************************************/

int
main(int argc, char* argv[])
{
#ifndef SKYHOOK_API_KEY
#  error SKYHOOK_API_KEY is undefined!
#endif

    const char* MY_API_KEY = SKYHOOK_API_KEY;

    SHLC_ReturnCode rc;
    SHLC_Location* location;
    const void* handle;

    handle = SHLC_init();
    if (! handle)
    {
        fprintf(stderr, "*** SHL_init failed!\n\n");
        return 1;
    }

    rc = SHLC_location(handle, MY_API_KEY, &location);
    if (rc != SHLC_OK)
    {
        fprintf(stderr, "*** SHLC_location failed (%d)!\n\n", rc);
    }
    else
    {
        print_location(location);
        SHLC_free_location(handle, location);
    }

    SHLC_deinit(handle);
    return 0;
}
