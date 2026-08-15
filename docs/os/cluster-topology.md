# Harmony OS Next — Cluster Topology: iBot Control Plane ↔ OS Effort

> All numbers below were **verified live on 2026-08-15** against the real,
> auth-gated APIs (Bearer `IBOT_TOKEN` from the project `.env`, token never
> printed). Sources:
> - `GET http://127.0.0.1:18789/api/cluster` (iBot Gateway)
> - `GET http://127.0.0.1:4003/health` (Python agents API)
> - `GET http://127.0.0.1:3000/api/nodes` (iBot UI, `requireAuthApi`)
> - `GET http://127.0.0.1:3000/api/skills/catalog` (iBot UI)
> - `find .agents/skills -name SKILL.md | wc -l` (on-disk packs)

---

## 1. Verified inventory (real numbers)

| Metric | Verified value | Source |
|---|---|---|
| Gateway cluster agents | **34** | `/api/cluster` → `agentCount: 34`, `agents[]` length 34, all `active: true` |
| — orchestrator tier | 4 | `by tier: {orchestrator: 4}` |
| — supervisor tier | 5 | `by tier: {supervisor: 5}` |
| — worker tier | 25 | `by tier: {worker: 25}` |
| Python agents | **9** | `:4003/health` → `{"ok":true,"service":"ibot-agents","version":"8.3.0-AGI","agents":9}` |
| Swarm nodes | **43** | `/api/nodes` → `nodes[]` length 43 |
| — node types | 4 ORCHESTRATOR, 9 SUPERVISOR, 30 WORKER | `/api/nodes` |
| — node sources | 34 `gateway` + 9 `python-agents` | `/api/nodes` |
| Skill catalog | **242 skills**, 15 categories, 6 featured | `/api/skills/catalog` → `catalog.skills` length 242 |
| Skill inventory on swarm nodes | 947 (per-node `skillCount` sum) | `/api/nodes` |
| On-disk skill packs | **685** `SKILL.md` | `find .agents/skills -name SKILL.md` |

> **Discrepancy note (honest reporting):** the task brief stated "36 gateway
> agents". The live gateway reports **34** (`CLUSTER_CONFIG` in
> `ibot-gateway/server.js` has 34 entries: 4 + 5 + 25). The docs use the
> verified value 34. `43 nodes` and `242 skills` match the brief exactly.

### Gateway agent model distribution (`/api/cluster` → `modelDistribution`)
| Ollama model | Count |
|---|---|
| `gemma3:12b` (orchestrators) | 4 |
| `llama3.1:8b` (supervisors) | 6 |
| `deepseek-r1:7b` (code/security workers) | 9 |
| `qwen2.5:7b` (data workers) | 5 |
| `mistral:7b` (devops/cloud workers) | 5 |
| `gemma3:4b` (incident-responder) | 1 |
| `llama3.2:1b` (fast workers) | 4 |
| **Total** | **34** |

### 43 swarm nodes (`/api/nodes` ids)
```
orchestrator, data-orchestrator, devops-orchestrator, security-orchestrator          (4 ORCHESTRATOR, gateway)
supervisor, data-supervisor, devops-supervisor, security-supervisor, design-agent    (5 SUPERVISOR, gateway)
design-agent-001, frontend-agent-001, backend-agent-001, qa-agent-001, deploy-agent-001,
aws-iam-agent-001, stackops-agent-001, cyberlab-agent-001, mining-agent-001          (9 python agents)
frontend-developer, backend-developer, code-builder, code-reviewer, code-auditor,
qa-engineer, ml-engineer, data-analyst, etl-worker, sql-agent, report-builder,
compliance-agent, devops-engineer, deploy-agent, cicd-agent, cloud-ops, container-ops,
vulnerability-scanner, pen-tester, incident-responder, executor, file-manager,
monitor-ops, network-ops, researcher                                              (25 WORKER, gateway)
```

---

## 2. How the mesh maps onto the OS development effort

| Swarm domain (agents) | OS subsystem it drives | Real kernel surface |
|---|---|---|
| orchestrator ×4, supervisor ×9 | kernel core, roadmap, build gates | `boot/boot.S`, `kernel/kmain_bare.c`, `kernel/{gdt,idt}.c`, `kernel/isr.s` |
| code-builder / code-reviewer / code-auditor | kernel core + syscall layer | `kernel/syscall.c`, `kernel/kprintf.c`, `kernel/string.c` |
| ml-engineer | QBM quantum scheduler, brain bridge | `quantum/qbm.c`, `quantum/scheduler_advisor.c`, `brain.cpp` |
| frontend-developer / design-agent | OS dashboard, noVNC view, HAL UI | `boot_vnc.sh`, `ws-vnc-bridge`, `dist/` |
| backend-developer | host bridge /proc-analog, node API | `bridge.c` → `/tmp/harmony_kernel_stats.json` |
| data-analyst / etl-worker / sql-agent | memory model + stats | `kernel/meminfo.c`, `kernel/memory.c` |
| devops-engineer / deploy-agent / cicd-agent / cloud-ops / container-ops | build pipeline, QEMU, ISO, containers | `build_baremetal_local.sh`, `Makefile`, `deploy/build_iso.sh`, `Dockerfile.baremetal` |
| vulnerability-scanner / pen-tester / incident-responder / compliance-agent | security sandbox | `security/access_control.c`, `kernel/syscall.c` (uid/caps), `SECURITY.md` |
| network-ops | network stack | `rust/rust_net.h`, gateway `:18789` as protocol reference |
| file-manager | filesystem model | `rust/rust_fs.h`, `kernel/syscall.c` fd table |
| monitor-ops | uptime/tick telemetry | `drivers/pit.c`, `kernel/shell.c` (`uptime`) |
| researcher | docs, evolution log, specs | this `docs/os/` set |

---

## 3. Control-plane services (started by `./start-all.sh`)

| Service | Port | Role in OS effort |
|---|---|---|
| iBot Gateway | 18789 | cluster agent hub; `/api/cluster` (34 agents); WS auth `IBOT_TOKEN` |
| Python Agents | 4003 | 9 FastAPI agents; `/health` reports `agents: 9`; agent registry for roles |
| iBot UI | 3000 | 43-node swarm graph + 242-skill catalog + noVNC viewer for the OS |
| Orchestrator | 4002 | task planner/executor the OS bridge reports into |
| Brain (Q-SINE) | 9008 | host-side neural engine; target of the kernel brain bridge |
| Mythos Stack | 18901 | telemetry/directives; can host kernel metrics |

---

## 4. Verification commands (repeatable, no secrets printed)

```bash
# gateway cluster (34 agents)
curl -s -H "Authorization: Bearer $IBOT_TOKEN" http://127.0.0.1:18789/api/cluster
# python agents (9)
curl -s http://127.0.0.1:4003/health
# swarm nodes (43) + skills catalog (242)
curl -s -H "Authorization: Bearer $IBOT_TOKEN" http://127.0.0.1:3000/api/nodes
curl -s -H "Authorization: Bearer $IBOT_TOKEN" http://127.0.0.1:3000/api/skills/catalog
# on-disk packs (685)
find .agents/skills -name SKILL.md | wc -l
```
