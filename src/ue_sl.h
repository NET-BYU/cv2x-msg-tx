/*
 *
 *  This file is part of the scientific research and development work conducted
 *  at the Communication Networks Institute (CNI), TU Dortmund University.
 *
 *  Copyright (C) 2021 Communication Networks Institute (CNI)
 *  Technische Universität Dortmund
 *
 *  Contact: kn.etit@tu-dortmund.de
 *  Authors: Fabian Eckermann
 *           fabian.eckermann@tu-dortmund.de
 *
 *  This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 * For more information on this software, see the institute's project website at:
 * http://www.cni.tu-dortmund.de
 *
 */

/*
 * Copyright 2013-2020 Software Radio Systems Limited
 *
 * This file is part of srsRAN.
 *
 * srsRAN is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsRAN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

/******************************************************************************
 *  File:         ue_sl.h
 *
 *  Description:  UE sidelink object.
 *
 *                This module is a frontend to all the sidelink data and control
 *                channel processing modules.
 *
 *  Reference:
 *****************************************************************************/

#ifndef SRSRAN_UE_SL_H
#define SRSRAN_UE_SL_H

#include <srsran/phy/ch_estimation/chest_sl.h>
// #include <srsran/phy/common/phy_common.h>
#include <srsran/phy/common/phy_common_sl.h>
#include <srsran/phy/dft/ofdm.h>
// #include <srsran/phy/phch/dci.h>
#include <srsran/phy/phch/pscch.h>
#include <srsran/phy/phch/pssch.h>
#include <srsran/phy/phch/ra_sl.h>
#include <srsran/phy/phch/sci.h>
// #include <srsran/phy/sync/cfo.h>
#include <srsran/phy/utils/debug.h>
#include <srsran/phy/utils/vector.h>

// #include "srsran/config.h"

//Manual override missing definitions 
// (this code was written for an older version of srsRAN and these were removed for later.)

#define SRSRAN_MAX_NUM_SUB_CHANNEL (20)

typedef struct SRSRAN_API {
  uint32_t tti;
} srsran_sl_sf_cfg_t;

typedef struct SRSRAN_API {
  uint8_t* ptr;

  uint32_t sub_channel_start_idx;
  uint32_t l_sub_channel;
} srsran_pssch_data_t;

// Resume regular code definitions

typedef struct SRSRAN_API {

  srsran_cell_sl_t cell;

  srsran_sl_comm_resource_pool_t sl_comm_resource_pool;

  srsran_ofdm_t fft[SRSRAN_MAX_PORTS];
  srsran_ofdm_t ifft;

  srsran_chest_sl_t pscch_chest_tx;
  srsran_chest_sl_t pssch_chest_tx;
  srsran_chest_sl_t pscch_chest_rx[SRSRAN_MAX_NUM_SUB_CHANNEL];
  srsran_chest_sl_t pssch_chest_rx[SRSRAN_MAX_NUM_SUB_CHANNEL];

  srsran_sci_t sci_rx[SRSRAN_MAX_NUM_SUB_CHANNEL];
  srsran_sci_t sci_tx;

  srsran_pscch_t pscch_tx;
  srsran_pssch_t pssch_tx;
  srsran_pscch_t pscch_rx[SRSRAN_MAX_NUM_SUB_CHANNEL];
  srsran_pssch_t pssch_rx[SRSRAN_MAX_NUM_SUB_CHANNEL];

  cf_t* signal_buffer_tx;
  cf_t* sf_symbols_tx;
  cf_t* signal_buffer_rx[SRSRAN_MAX_CHANNELS];
  cf_t* sf_symbols_rx[SRSRAN_MAX_PORTS];
  cf_t* equalized_sf_buffer;

  uint32_t nof_rx_antennas;
  uint32_t sf_len;
  uint32_t sf_n_re;

} srsran_ue_sl_t;

typedef struct SRSRAN_API {
  srsran_sci_t sci[SRSRAN_MAX_NUM_SUB_CHANNEL];
  uint8_t*     data[SRSRAN_MAX_NUM_SUB_CHANNEL];
} srsran_ue_sl_res_t;

SRSRAN_API int srsran_ue_sl_init(srsran_ue_sl_t* q,
                                 srsran_cell_sl_t cell,
                                 srsran_sl_comm_resource_pool_t sl_comm_resource_pool,
                                 uint32_t nof_rx_antennas);

SRSRAN_API void srsran_ue_sl_free(srsran_ue_sl_t* q);

SRSRAN_API int srsran_ue_sl_set_cell(srsran_ue_sl_t* q, srsran_cell_sl_t cell);

SRSRAN_API int srsran_ue_sl_set_sl_comm_resource_pool(srsran_ue_sl_t* q, srsran_sl_comm_resource_pool_t sl_comm);

SRSRAN_API uint32_t srsran_n_x_id_from_crc(uint8_t *crc, uint32_t crc_len);

SRSRAN_API void srsran_set_sci(srsran_sci_t* sci,
                               uint32_t priority,
                               uint32_t resource_reserv_itvl,
                               uint32_t time_gap,
                               bool     retransmission,
                               uint32_t transmission_format,
                               uint32_t mcs_idx);

SRSRAN_API void srsran_set_sci_riv(srsran_ue_sl_t* q,
                                   uint32_t sub_channel_start_idx,
                                   uint32_t l_sub_channel);

SRSRAN_API int srsran_ue_sl_encode(srsran_ue_sl_t* q,
                                   srsran_sl_sf_cfg_t* sf,
                                   srsran_pssch_data_t* data);

SRSRAN_API int srsran_ue_sl_decode_fft_estimate(srsran_ue_sl_t* q);

SRSRAN_API int srsran_ue_sl_decode_subch(srsran_ue_sl_t* q,
                                         srsran_sl_sf_cfg_t* sf,
                                         uint32_t sub_channel_idx,
                                         srsran_ue_sl_res_t* sl_res);


#endif // SRSRAN_UE_SL_H