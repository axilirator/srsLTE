/*
 * Copyright 2020 Software Radio Systems Limited
 * Author: Vadim Yanitskiy <axilirator@gmail.com>
 * Sponsored by Positive Technologies
 *
 * This file is part of srsLTE.
 *
 * srsLTE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsLTE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#include <stdint.h>
#include <asm/byteorder.h>

#include "srslte/common/common.h"
#include "srslte/interfaces/ue_interfaces.h"

namespace rrctl {

namespace proto {

  enum msg_type {
    RRCTL_RESET = 0x00,
    RRCTL_DATA,
    RRCTL_PLMN_SEARCH,
    RRCTL_PLMN_SELECT,
    RRCTL_CONN_ESTABLISH,
    RRCTL_CONN_RELEASE,
    RRCTL_PAGING,
  };

  enum msg_disc {
    RRCTL_REQ = 0x00,
    RRCTL_IND = 0x01,
    RRCTL_CNF = 0x02,
    RRCTL_ERR = 0x03,
  };

  struct msg_hdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
    uint8_t disc:2, type:6;
#elif defined(__BIG_ENDIAN_BITFIELD)
    uint8_t type:6, disc:2;
#else
    #error "Please fix <asm/byteorder.h>"
#endif
    uint8_t rfu; // Reserved
    uint16_t len;
    uint8_t data[0];
  } __attribute__((packed));

  struct msg_plmn_search_res {
    uint8_t nof_plmns;
    struct plmn {
      uint8_t mcc[2];
      uint8_t mnc[2];
      uint16_t tac;
    } plmns[16];
  } __attribute__((packed));

  struct msg_plmn_select_req {
    uint8_t mcc[2];
    uint8_t mnc[2];
  } __attribute__((packed));

  struct msg_conn_establish_req {
    uint8_t cause;
    uint8_t pdu[0];
  } __attribute__((packed));

  struct msg_data {
    uint32_t lcid;
    uint8_t pdu[0];
  } __attribute__((packed));

  struct __mmec_m_tmsi {
    uint8_t mmec;
    uint32_t m_tmsi;
  } __attribute__((packed));

  struct msg_paging_ind {
    struct __mmec_m_tmsi ueid;
  } __attribute__((packed));

  struct msg {
    struct msg_hdr hdr;
    union {
      struct msg_data data;
      struct msg_paging_ind paging_ind;
      struct msg_plmn_search_res plmn_search_res;
      struct msg_plmn_select_req plmn_select_req;
      struct msg_conn_establish_req conn_establish_req;
    } u;
  } __attribute__((packed));

  std::string msg_hdr_desc(proto::msg_type type, proto::msg_disc disc, uint16_t len = 0);

} // namespace proto

namespace codec {

class error : public std::runtime_error {
public:
  explicit error(const std::string& msg) : std::runtime_error(msg) {};
};

void enc_hdr(srslte::byte_buffer_t& buf,
             proto::msg_type type,
             proto::msg_disc disc,
             uint16_t len = 0);
const uint8_t* dec_hdr(const srslte::byte_buffer_t& buf,
                       proto::msg_type& type,
                       proto::msg_disc& disc,
                       uint16_t& len);

void enc_plmn_search_res(srslte::byte_buffer_t& buf,
                         const srsue::rrc_interface_nas::found_plmn_t* plmns,
                         size_t nof_plmns);

void dec_plmn_select_req(std::pair<uint16_t, uint16_t>& mcc_mnc,
                         const uint8_t* payload, size_t len);

void dec_conn_establish_req(srslte::establishment_cause_t& cause,
                            const uint8_t*& pdu, size_t& pdu_len,
                            const uint8_t* payload, size_t len);

void enc_data_ind(srslte::byte_buffer_t& buf,
                  const uint8_t *pdu, size_t pdu_len,
                  uint32_t lcid);

void enc_paging_ind(srslte::byte_buffer_t& buf,
                    srslte::s_tmsi_t* ue_identity);

} // namespace codec

} // namespace rrctl
