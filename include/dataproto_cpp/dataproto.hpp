// Modified dataproto sources from locus-game, credit to BlobKat for the implementation, based off
// the original npm project, dataproto https://www.npmjs.com/package/dataproto?activeTab=readme.
#pragma once

#include <cstdint>
#include <cstring>
#include <string>

using namespace std;
namespace dataproto {
	// Print an error message and abort
	using FailFunction = void (*)(const void *reason);
    void set_fail_function(FailFunction func);

	// Buffer writer that gracefully handles out-of-bounds by automatically expanding the allocated buffer
	struct BufWriter {
	public:
		union f2u32 {
			float _;
			uint32_t value;
		};
		union f2u64 {
			double _;
			uint64_t value;
		};
		uint8_t *start, *end, *head;
		~BufWriter();
		size_t size() const;
		size_t capacity() const;
		BufWriter();
		BufWriter(BufWriter &&a);
		BufWriter(BufWriter &a) = delete;
		BufWriter &operator=(BufWriter a) = delete;
		BufWriter &operator=(BufWriter &a) = delete;
		void _expand();
		void skip(size_t n);
		void flint(uint32_t n);
		void flint16(uint16_t n);
		void u8(uint8_t n);
		void i8(int8_t n);
		void u16(uint16_t n);
		void i16(int16_t n);
		void u32(uint32_t n);
		void i32(int32_t n);
		void u64(uint64_t n);
		void i64(uint64_t n);
		void f32(float n);
		void f64(double n);
		void str(const void *s, size_t n);
		void arr(const void *s, size_t n);
		void str(const string &s);
		void str(string &&s);
		void str(char *s);
		void arr(const string &s);
		void arr(string &&s);
		operator string() const;
		const char* data() const;
	};

	// Buffer reader that gracefully handles out-of-bounds by returning 0
	struct BufReader {
	public:
		union u2f32 {
			uint32_t _;
			float value;
		};
		union u2f64 {
			uint64_t _;
			double value;
		};
		uint8_t *head;
		uint8_t *end;
		BufReader(char *dat, size_t size);
		static BufReader c_str(char *dat);
		BufReader(string &dat);
		size_t left() const;
		void skip(size_t n);
		uint32_t flint();
		uint16_t flint16();
		uint8_t u8();
		int8_t i8();
		uint16_t u16();
		int16_t i16();
		uint32_t u32();
		int32_t i32();
		uint64_t u64();
		int64_t i64();
		float f32();
		double f64();
		struct slice {
			void *data;
			size_t size;
			operator string();
			operator string_view();
			operator void *();
			template <typename T>
			explicit operator T *();
			char *copy();
			char *copy(void *a);
		};
		slice str();
		slice arr(size_t n);
		string remaining() const;
		bool overran() const;
	};
}