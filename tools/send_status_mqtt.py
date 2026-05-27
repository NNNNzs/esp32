#!/usr/bin/env python3
import argparse
import json
import os
import socket
import sys
import time


def encode_remaining_length(length):
    out = bytearray()
    while True:
        digit = length % 128
        length //= 128
        if length:
            digit |= 0x80
        out.append(digit)
        if not length:
            return bytes(out)


def encode_string(value):
    data = value.encode("utf-8")
    return len(data).to_bytes(2, "big") + data


def mqtt_connect(client_id):
    payload = bytearray()
    payload.extend(encode_string("MQTT"))
    payload.append(4)
    payload.append(2)
    payload.extend((0, 30))
    payload.extend(encode_string(client_id))
    return bytes([0x10]) + encode_remaining_length(len(payload)) + bytes(payload)


def mqtt_publish(topic, payload):
    body = bytearray()
    body.extend(encode_string(topic))
    body.extend(payload.encode("utf-8"))
    return bytes([0x30]) + encode_remaining_length(len(body)) + bytes(body)


def main():
    parser = argparse.ArgumentParser(description="Publish a Claude Code status over MQTT.")
    parser.add_argument("status", choices=["idle", "thinking", "approval", "working", "done", "error"])
    parser.add_argument("--host", default=os.environ.get("CLAUDE_MQTT_HOST", "127.0.0.1"))
    parser.add_argument("--port", type=int, default=int(os.environ.get("CLAUDE_MQTT_PORT", "1883")))
    parser.add_argument("--topic", default=os.environ.get("CLAUDE_MQTT_TOPIC", "claudecode/device/default/status"))
    parser.add_argument("--client-id", default=os.environ.get("CLAUDE_MQTT_CLIENT_ID", "claudecode-hook"))
    args = parser.parse_args()

    payload = json.dumps({"state": args.status, "ts": int(time.time())}, separators=(",", ":"))

    with socket.create_connection((args.host, args.port), timeout=5) as sock:
        sock.sendall(mqtt_connect(args.client_id))
        ack = sock.recv(4)
        if len(ack) < 4 or ack[0] != 0x20 or ack[3] != 0:
            raise SystemExit("MQTT CONNECT failed")
        sock.sendall(mqtt_publish(args.topic, payload))
        sock.sendall(b"\xE0\x00")


if __name__ == "__main__":
    sys.exit(main())
