def count_hooked_lines(log_file_path):
    count = 0
    with open(log_file_path, 'r', encoding='utf-8') as file:
        for line in file:
            if "hooked" in line:
                count += 1
    return count

# Example usage
log_path = "./txt-fr2.log"
hooked_count = count_hooked_lines(log_path)
print(f"Lines containing 'hooked': {hooked_count}")
