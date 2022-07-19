/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2022, Ideas on Board Oy
 *
 * jpeg_error_manager.cpp - JPEG Error Manager
 */

#include "jpeg_error_manager.h"

static void errorExit(j_common_ptr cinfo) {
  JpegErrorManager* self = reinterpret_cast<JpegErrorManager*>(cinfo->err);
  longjmp(self->escape_, 1);
}

static void outputMessage([[maybe_unused]] j_common_ptr cinfo) {}

JpegErrorManager::JpegErrorManager(struct jpeg_decompress_struct& cinfo) {
  cinfo.err = jpeg_std_error(&errmgr_);
  errmgr_.error_exit = errorExit;
  errmgr_.output_message = outputMessage;
}