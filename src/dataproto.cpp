#include <cstring>

#include "../include/dataproto_cpp/dataproto.hpp"

using namespace std;
namespace dataproto {
	// Print an error message and abort
	void default_fail(const void *reason) {
		fprintf(stderr, "dataproto error: %s\n", static_cast<const char*>(reason));
		abort();
	}
	FailFunction fail = default_fail;
	void set_fail_function(FailFunction failFunction) {
		fail = failFunction;
	}

	// BufWriter
	BufWriter::~BufWriter() {
		free(this->start);
	}
	size_t BufWriter::size() const {
		return (size_t)(this->head - this->start);
	}
	size_t BufWriter::capacity() const {
		return (size_t)(this->end - this->start);
	}
	BufWriter::BufWriter() {
		this->start = this->head = (uint8_t *)malloc(16);
		this->end = this->start + 16;
	}
	BufWriter::BufWriter(BufWriter &&a) {
		this->start = a.start;
		this->end = a.end;
		this->head = a.head;
		a.start = a.head = a.end = nullptr;
	}
	void BufWriter::_expand() {
		auto sz = (size_t)(this->head - this->start), nsz = (size_t)(this->end - this->start) << 1;
		if (!(this->start = (uint8_t *)realloc(this->start, nsz)))
			fail("Out of memory while reallocating Buffer");
		this->end = this->start + nsz;
		this->head = this->start + sz;
	}
	void BufWriter::skip(size_t n) {
		if (this->head > this->end - n) {
			auto sz = (size_t)(this->head - this->start), nsz = ((size_t)(this->end - this->start) << 1) + n;
			if (!(this->start = (uint8_t *)realloc(this->start, nsz)))
				fail("Out of memory while reallocating Buffer");
			this->end = this->start + nsz;
			this->head = (this->start + sz) + n;
		} else
			this->head += n;
	}
	// Store a signed int in [0,2147483647] using fewer bytes for smaller numbers
	// 0-63             -> 00xxxxxx
	// 64-16383         -> 01xxxxxx xxxxxxxx
	// 16384-2147483647 -> 1xxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx
	void BufWriter::flint(uint32_t n) {
		if (this->head > this->end - 4)
			this->_expand();
		if (n < 64)
			*this->head++ = n;
		else if (n < 16384)
			*this->head = n >> 8 | 0x40, this->head[1] = n, this->head += 2;
		else
			*this->head = n >> 24 | 0x80, this->head[1] = n >> 16, this->head[2] = n >> 8, this->head[3] = n, this->head += 4;
	}
	void BufWriter::flint16(uint16_t n) {
		if (this->head > this->end - 2)
			this->_expand();
		if (n < 128)
			*this->head++ = n;
		else
			*this->head = n >> 8 | 0x80, this->head[1] = n, this->head += 2;
	}
	void BufWriter::u8(uint8_t n) {
		if (this->head == this->end)
			this->_expand();
		*this->head++ = n;
	}
	void BufWriter::i8(int8_t n) { u8(n); }
	void BufWriter::u16(uint16_t n) {
		if (this->head >= this->end - 1)
			this->_expand();
		*this->head = n >> 8;
		this->head[1] = n;
		this->head += 2;
	}
	void BufWriter::i16(int16_t n) { u16(n); }
	void BufWriter::u32(uint32_t n) {
		if (this->head > this->end - 4)
			this->_expand();
		*this->head = n >> 24;
		this->head[1] = n >> 16;
		this->head[2] = n >> 8;
		this->head[3] = n;
		this->head += 4;
	}
	void BufWriter::i32(int32_t n) { u32(n); }
	void BufWriter::u64(uint64_t n) {
		if (this->head > this->end - 8)
			this->_expand();
		*this->head = n >> 56;
		this->head[1] = n >> 48;
		this->head[2] = n >> 40;
		this->head[3] = n >> 32;
		this->head[4] = n >> 24;
		this->head[5] = n >> 16;
		this->head[6] = n >> 8;
		this->head[7] = n;
		this->head += 8;
	}
	void BufWriter::i64(uint64_t n) { u64(n); }
	void BufWriter::f32(float n) { u32(f2u32{ n }.value); }
	void BufWriter::f64(double n) { u64(f2u64{ n }.value); }
	void BufWriter::str(const void *s, size_t n) {
		if (this->head > this->end - n - 4) {
			auto sz = (size_t)(this->head - this->start), nsz = ((this->end - this->start) << 1) + n;
			if (!(this->start = (uint8_t *)realloc(this->start, nsz)))
				fail("Out of memory while reallocating Buffer");
			this->end = this->start + nsz;
			this->head = this->start + sz;
		}
		if (n < 64)
			*this->head++ = n;
		else if (n < 16384)
			*this->head = n >> 8 | 0x40, this->head[1] = n, this->head += 2;
		else
			*this->head = n >> 24 | 0x80, this->head[1] = n >> 16, this->head[2] = n >> 8, this->head[3] = n, this->head += 4;
		memcpy(this->head, s, n);
		this->head += n;
	}
	void BufWriter::arr(const void *s, size_t n) {
		if (this->head > this->end - n) {
			auto sz = (size_t)(this->head - this->start), nsz = ((this->end - this->start) << 1) + n;
			if (!(this->start = (uint8_t *)realloc(this->start, nsz)))
				fail("Out of memory while reallocating Buffer");
			this->end = this->start + nsz;
			this->head = this->start + sz;
		}
		memcpy(this->head, s, n);
		this->head += n;
	}
	void BufWriter::str(const string &s) { str(s.data(), s.size()); }
	void BufWriter::str(string &&s) { str(s.data(), s.size()); }
	void BufWriter::str(char *s) { str(s, strlen(s)); }
	void BufWriter::arr(const string &s) { arr(s.data(), s.size()); }
	void BufWriter::arr(string &&s) { arr(s.data(), s.size()); }
	BufWriter::operator string() const {
		return string((char *)this->start, (size_t)(this->head - this->start));
	}
	const char* BufWriter::data() const { return (char *)this->start; }

	// BufReader
	BufReader::BufReader(char* data, size_t size) {
		this->head = (uint8_t *)data;
		this->end = this->head + size;
	}
	BufReader BufReader::c_str(char *dat) { return BufReader(dat, strlen(dat)); }
	BufReader::BufReader(string &dat) {
		this->head = (uint8_t *)dat.data();
		this->end = this->head + dat.size();
	}
	size_t BufReader::left() const { return (size_t)(this->end - this->head); }
	void BufReader::skip(size_t n) { this->head += n; }
	// Load a signed int in [0,2147483647] reading fewer bytes for smaller numbers
	// 00xxxxxx -> 0-63
	// 01xxxxxx xxxxxxxx -> 0-16383
	// 1xxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx -> 0-2147483647
	uint32_t BufReader::flint() {
		if (this->head >= this->end)
			return (this->head++, 0);
		uint8_t n = *this->head++;
		if (n < 64)
			return n;
		else if (n < 128)
			return this->head >= this->end ? (this->head++, 0) : (n & 0x3F) << 8 | (*this->head++);
		else
			return this->head > this->end - 3 ? (this->head += 3, 0) : (n & 0x7F) << 24 | (*this->head++) << 16 | (*this->head++) << 8 | (*this->head++);
	}
	uint16_t BufReader::flint16() {
		if (this->head >= this->end)
			return (this->head++, 0);
		uint8_t n = *this->head++;
		if (n < 128)
			return n;
		else
			return this->head >= this->end ? (this->head++, 0) : (n & 0x7F) << 8 | (*this->head++);
	}
	uint8_t BufReader::u8() {
		uint8_t a = this->head >= this->end ? 0 : *this->head;
		this->head++;
		return a;
	}
	int8_t BufReader::i8() { return u8(); }
	uint16_t BufReader::u16() {
		uint16_t a = this->head >= this->end - 1 ? 0 : *this->head << 8 | this->head[1];
		this->head += 2;
		return a;
	}
	int16_t BufReader::i16() { return u16(); }
	uint32_t BufReader::u32() {
		uint32_t a = this->head > this->end - 4 ? 0 : *this->head << 24 | this->head[1] << 16 | this->head[2] << 8 | this->head[3];
		this->head += 4;
		return a;
	}
	int32_t BufReader::i32() { return u32(); }
	uint64_t BufReader::u64() {
		uint64_t a = this->head > this->end - 8 ? 0 : uint64_t(*this->head << 24 | this->head[1] << 16 | this->head[2] << 8 | this->head[3]) << 32 | (this->head[4] << 24 | this->head[5] << 16 | this->head[6] << 8 | this->head[7]);
		this->head += 8;
		return a;
	}
	int64_t BufReader::i64() { return u64(); }
	float BufReader::f32() { return u2f32{ u32() }.value; }
	double BufReader::f64() { return u2f64{ u64() }.value; }
	// slice
	BufReader::slice::operator string() { return string((char *)data, size); }
	BufReader::slice::operator string_view() { return string_view((char *)data, size); }
	BufReader::slice::operator void *() { return this->data; }
	template <typename T>
	BufReader::slice::operator T *() { return (T *)this->data; }
	char *BufReader::slice::copy() {
		if (!this->size)
			return 0;
		char *x = new char[this->size];
		memcpy(x, this->data, this->size);
		return x;
	}
	char *BufReader::slice::copy(void *a) {
		if (this->size)
			;
		memcpy(a, this->data, this->size);
		return (char *)a;
	}

	BufReader::slice BufReader::str() {
		if (this->head >= this->end)
			return { ++this->head, 0 };
		uint8_t n = *this->head++;
		if (n >= 64) {
			if (n < 128)
				n = this->head >= this->end ? (this->head++, 0) : (n & 0x3F) << 8 | (*this->head++);
			else
				n = this->head > this->end - 3 ? (this->head += 3, 0) : (n & 0x7F) << 24 | (*this->head++) << 16 | (*this->head++) << 8 | (*this->head++);
		}
		auto s = this->head;
		this->head += n;
		return { s, n };
	}
	BufReader::slice BufReader::arr(size_t n) {
		if (this->head > this->end - n)
			return slice{ this->head, 0 };
		auto s = this->head;
		this->head += n;
		return slice{ s, n };
	}
	string BufReader::remaining() const {
		return string((char *)this->head, (size_t)(this->end - this->head));
	}
	bool BufReader::overran() const {
		return this->head > this->end;
	}
}