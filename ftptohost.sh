#!/usr/bin/env bash
#set -x

set -euo pipefail

if [[ $# -lt 2 ]]; then
    echo "Usage: $0 credentials_file file1 [file2 ...]" >&2
    exit 1
fi

CREDENTIALS_FILE="$1"
shift

if [[ ! -f "$CREDENTIALS_FILE" ]]; then
    echo "ERROR: Credentials file '$CREDENTIALS_FILE' not found." >&2
    exit 1
fi

# Load credentials
# shellcheck disable=SC1090
source "$CREDENTIALS_FILE"

: "${FTP_HOST:?FTP_HOST is not set in $CREDENTIALS_FILE}"
: "${FTP_USER:?FTP_USER is not set in $CREDENTIALS_FILE}"
: "${FTP_PASSWORD:?FTP_PASSWORD is not set in $CREDENTIALS_FILE}"

FTP_REMOTE_DIR="${FTP_REMOTE_DIR:-/}"

# Check that all files exist before uploading anything
for file in "$@"; do
    if [[ ! -f "$file" ]]; then
        echo "ERROR: File not found: $file" >&2
        exit 1
    fi
done

for file in "$@"; do
    filename=$(basename "$file")
    url="ftp://${FTP_HOST}${FTP_REMOTE_DIR%/}/$filename"

    # echo "Uploading $file -> $url"

    curl \
        --fail \
        --silent \
        --show-error \
        --use-ascii \
        --user "$FTP_USER:$FTP_PASSWORD" \
        --upload-file "$file" \
        "$url"

    echo "Uploaded $file"
done

#echo "All files uploaded successfully."