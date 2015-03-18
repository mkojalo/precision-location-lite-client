/*
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

#include "spi/CellAdapter.h"

namespace WPS {
namespace SPI {

/*********************************************************************/
/*                                                                   */
/* StaticCellAdapter                                                 */
/*                                                                   */
/* NOTE: this adapter returns static data and is for testing         */
/*       purposes only                                               */
/*                                                                   */
/*********************************************************************/

class StaticCellAdapter
    : public CellAdapter
{
public:

    StaticCellAdapter()
        : _listener(NULL)
    {}

    virtual std::string description() const
    {
        return "StaticCellAdapter";
    }

    virtual void setListener(Listener* listener)
    {
        _listener = listener;
    }

    virtual ErrorCode open()
    {
        std::vector<ScannedCellTower> cells;
        cells.push_back(ScannedCellTower(CellTower::GSMTower(310, 260, 60803, 36489), 1, -50));
        _listener->onCellChanged(cells);
        return SPI_OK;
    }

    virtual void close()
    {}

    virtual ErrorCode getIMEI(std::string& imei)
    {
        imei = "01234567890123";
        return SPI_OK;
    }

private:

    Listener* _listener;
};

/*********************************************************************/
/*                                                                   */
/* CellAdapter::newInstance                                          */
/*                                                                   */
/*********************************************************************/

CellAdapter*
CellAdapter::newInstance()
{
    return new StaticCellAdapter();
}

}
}
