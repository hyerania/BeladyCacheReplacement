import re
import pandas as pd
from collections import OrderedDict
def parse_line(line):
    parts = line.split(' -> ')
    if len(parts) != 2:
        return None  # or handle the error as needed

    address = parts[0]
    details = parts[1].split(', ')
    if len(details) < 2:
        return None  # or handle the error as needed

    type_part = details[0].split(': ')
    if len(type_part) != 2:
        return None  # or handle the error as needed

    way_type = type_part[0]

    try:
        way_number = int(type_part[1])
        set_value = int(details[1].split(': ')[1])
    except ValueError:
        return None  # or handle the error as needed

    return address, way_type, way_number, set_value

data1 = []
with open('/mnt/e/F3CAS/Hawkeye/Test_Hawkeye/VictimWays.txt', 'r') as file:
    for line in file:
        parsed_line = parse_line(line.strip())
        if parsed_line:
            data1.append(parsed_line)

df_CPP = pd.DataFrame(data1, columns=['Address', 'Type', 'TypeNumber', 'Set'])

print("-"*50)

data2 = []
with open('/mnt/e/F3CAS/Python-code/Hawkeye/VictimWaysfinal.txt', 'r') as file2:
    for line in file2:
        parsed_line = parse_line(line.strip())
        if parsed_line:
            data2.append(parsed_line)  # Corrected to append to data2

df_python = pd.DataFrame(data2, columns=['Address', 'Type', 'TypeNumber', 'Set'])

print("*"*50)
# Check structure and labels
print("Python DataFrame Structure:")
print(df_python.info())
print("CPP DataFrame Structure:")
print(df_CPP.info())

# Now perform the comparison if both DataFrames are correctly structured
if not df_python.empty and not df_CPP.empty:
    if df_python.equals(df_CPP):
        print("Python and C++ outputs are equal")
    else:
        print("Python and C++ outputs are Not equal")
        differences = df_python.compare(df_CPP, keep_shape=True, keep_equal=True)
        print("Differences:", differences)
