#pragma once

#include <d3d11.h>
#include <unordered_set>

namespace themmokhtar
{
	namespace d3d11
	{
		namespace fingerprint
		{
			struct ModelFingerprint
			{
				UINT vertexStride;
				UINT vertexByteWidth;
				UINT indexByteWidth;
				UINT constantByteWidth;
			};
			bool operator==(const ModelFingerprint& lhs, const ModelFingerprint& rhs);
			template<> struct ::std::hash<themmokhtar::d3d11::fingerprint::ModelFingerprint>;

			class FingerprintController
			{
				static ModelFingerprint getModelFingerprint(ID3D11DeviceContext* context);

			public:
				FingerprintController();
				~FingerprintController();

				bool captureFingerprint(ID3D11DeviceContext* context);
				void clearFingerprints();

				SIZE_T getFingerprintCount() const;
				bool hasFingerprint(ID3D11DeviceContext* context) const;
				ModelFingerprint getFingerprintAt(SIZE_T index) const;

			private:
				std::unordered_set<ModelFingerprint> *fingerprints;
				
				
			};
		} // namespace fingerprint
	} // namespace d3d11
} // namespace tmml