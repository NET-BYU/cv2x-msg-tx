import scipy.io
from xml_to_dict import XMLtoDict
from pprint import pprint

parser = XMLtoDict()
data = scipy.io.loadmat("2023-06-29 OBU message at 5.915.mat")

# metadata = data["rsaMetadata"][0]
# metadata = parser.parse(data["rsaMetadata"][0])
# metadata = metadata["DataFile"] # ["Setup"]["RSAPersist"]

# print(metadata.keys())
# pprint(metadata)


iq = data["Y"].flatten()

# print(iq.dtype)

iq.tofile("2023-06-29_OBU.cf64")