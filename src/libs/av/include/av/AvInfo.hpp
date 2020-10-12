/*
 * Copyright (C) 2015 Emeric Poupon
 *
 * This file is part of LMS.
 *
 * LMS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LMS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LMS.  If not, see <http://www.gnu.org/licenses/>.
 */

/* This file contains some classes in order to get info from file using the libavconv */

#pragma once

#include <chrono>
#include <filesystem>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "AvTypes.hpp"

struct AVFormatContext;

namespace Av
{

void AvInit();

struct Picture
{
	std::string mimeType;
	const std::byte* data {};
	std::size_t dataSize;
};

struct StreamInfo
{
	size_t		id;
	std::size_t     bitrate;
};

class MediaFileException : public AvException
{
	public:
		MediaFileException(int avError);
};

class MediaFile
{
	public:
		MediaFile(const std::filesystem::path& p);
		~MediaFile();

		MediaFile(const MediaFile&) = delete;
		MediaFile& operator=(const MediaFile&) = delete;
		MediaFile(MediaFile&&) = delete;
		MediaFile& operator=(MediaFile&&) = delete;

		std::string		getFormatName() const;

		const std::filesystem::path&		getPath() const {return _p;};

		std::chrono::milliseconds		getDuration() const;
		std::map<std::string, std::string>	getMetaData(void);

		std::vector<StreamInfo>	getStreamInfo() const;
		std::optional<std::size_t>	getBestStream() const; // none if failure/unknown
		bool			hasAttachedPictures(void) const;
		void			visitAttachedPictures(std::function<void(const Picture&)> func) const;

	private:

		const std::filesystem::path	_p;
		AVFormatContext* _context {};
};


struct MediaFileFormat
{
	std::string mimeType;
	std::string format;
};

std::optional<MediaFileFormat> guessMediaFileFormat(const std::filesystem::path& file);

} // namespace Av

