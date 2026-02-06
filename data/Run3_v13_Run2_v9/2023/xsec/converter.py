import json
import csv

# 1. JSON 파일 불러오기
with open('WREE.json', 'r') as f:
    data = json.load(f)

# 2. CSV 파일 저장 설정
output_file = 'WREExsec_summary.csv'
header = ['Short_Name', 'Cross_Section_pb', 'Uncertainty_pb', 'Status', 'MCM', 'DAS']

with open(output_file, 'w', newline='', encoding='utf-8') as f:
    writer = csv.writer(f)
    writer.writerow(header)
    
    for entry in data:
        # _TuneCP5 앞부분만 샘플 이름으로 추출
        full_name = entry.get('process_name', '')
        short_name = full_name.split('_TuneCP5')[0]
        
        # 필요한 값들 가져오기
        xsec = entry.get('cross_section', 0)
        unc = entry.get('total_uncertainty', 0)
        status = entry.get('status', 'N/A')
        mcm = entry.get('MCM', 'N/A')
        das = entry.get('DAS', 'N/A')
        
        # 행 추가
        writer.writerow([short_name, xsec, unc, status, mcm, das])

print(f">> 변환 완료! '{output_file}' 파일을 확인하세요.")


# 1. JSON 파일 불러오기
with open('WRMM.json', 'r') as f:
    data = json.load(f)

# 2. CSV 파일 저장 설정
output_file = 'WRMMxsec_summary.csv'
header = ['Short_Name', 'Cross_Section_pb', 'Uncertainty_pb', 'Status', 'MCM', 'DAS']

with open(output_file, 'w', newline='', encoding='utf-8') as f:
    writer = csv.writer(f)
    writer.writerow(header)
    
    for entry in data:
        # _TuneCP5 앞부분만 샘플 이름으로 추출
        full_name = entry.get('process_name', '')
        short_name = full_name.split('_TuneCP5')[0]
        
        # 필요한 값들 가져오기
        xsec = entry.get('cross_section', 0)
        unc = entry.get('total_uncertainty', 0)
        status = entry.get('status', 'N/A')
        mcm = entry.get('MCM', 'N/A')
        das = entry.get('DAS', 'N/A')
        
        # 행 추가
        writer.writerow([short_name, xsec, unc, status, mcm, das])

print(f">> 변환 완료! '{output_file}' 파일을 확인하세요.")